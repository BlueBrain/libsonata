#include <dirent.h>
#include <stdlib.h>
#include <new>
#include <iostream>
#include <stdlib.h>

#include "reportinglib.hpp"
/*#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>*/

double ReportingLib::m_atomicStep = 1e-8;
MPI_Comm ReportingLib::m_allCells = MPI_COMM_WORLD;
int ReportingLib::m_rank = 0;

ReportingLib::ReportingLib(): m_numReports(0) {

    /*std::cout << "Creating ReportingLib class" << std::endl;
    mkdir("./logs", 0777);
    const std::string logger_name ="ReportingLib";
    m_logger = spdlog::get(logger_name);
    if (!m_logger) {
        m_logger = spdlog::rotating_logger_mt(logger_name, "./logs/test.log", 1048576 * 5, 3);
        m_logger->set_level(spdlog::level::level_enum::trace);
    }*/
}

ReportingLib::~ReportingLib() {
    clear();
}

void ReportingLib::clear() {

    for (auto &kv: m_reports) {
        //m_logger->info("Deleting report: {}", kv.first);
        //std::cout << "Deleting " << kv.first << std::endl;
    }
    m_reports.clear();
}

bool ReportingLib::is_empty() {
    return (!m_numReports);
}

int ReportingLib::get_num_reports() {
    return m_numReports;
}

KindReport ReportingLib::string_to_enum(const std::string& kind) {

    if(kind == "compartment") {
        return COMPARTMENT;
    } else if (kind == "soma") {
        return SOMA;
    } else if (kind == "spike") {
        return SPIKE;
    }
}

int ReportingLib::add_report(const std::string& report_name, int cellnumber, unsigned long gid, unsigned long vgid,
                             double tstart, double tend, double dt,const std::string& kind) {

    // check if this is the first time a Report with the given name is referenced
    ReportMap::iterator reportFinder = m_reports.find(report_name);

    std::shared_ptr<Report> report;
    if (reportFinder != m_reports.end()) {
        report = reportFinder->second;
        std::cout << "Report '" << report_name << "' found!" << std::endl;
    } else {
        KindReport kind_report = string_to_enum(kind);
        // new report
        report = Report::createReport(report_name, tstart, tend, dt, kind_report);
        // Check if kind doesnt exist
        if(report) {
            m_reports[report_name] = report;
            m_numReports++;
            //std::cout << "Creating new report: " << report_name << std::endl;
            //std::cout << "Total number of reports: " << m_numReports << std::endl;
        }
    }

    if (report) {
        report->add_cell(cellnumber, gid, vgid);
    }
    return 0;
}

int ReportingLib::add_variable(const std::string& report_name, int cellnumber, double* pointer) {

    ReportMap::iterator reportFinder = m_reports.find(report_name);

    if (reportFinder != m_reports.end()) {
        return reportFinder->second->add_variable(cellnumber, pointer);
    }

    return 1;
}

void ReportingLib::make_global_communicator() {

    int global_rank, global_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &global_size);
    std::cout << "++++++Num reports for rank " << global_rank << " is " << m_numReports<< std::endl;

    // Create a second communicator where
    MPI_Comm_split(MPI_COMM_WORLD, m_numReports == 0, 0, &ReportingLib::m_allCells);

    int cell_rank, cell_size;
    MPI_Comm_rank(ReportingLib::m_allCells, &cell_rank);
    MPI_Comm_size(ReportingLib::m_allCells, &cell_size);

    printf("+++++++++WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n",
           global_rank, global_size, cell_rank, cell_size);
}

void ReportingLib::share_and_prepare(int rank, int num_nodes) {

    m_rank = rank;
    // Split reports into different ranks?

    // Create communicator groups ?

    // remove reports without cells
    for(auto& kv: m_reports) {
        if(kv.second->is_empty()){
            m_reports.erase(kv.first);
        }
    }

    // Allocate buffers ?
    std::cout << "========+++++ m_rank is " << m_rank << std::endl;
    for (auto &kv : m_reports) {
        std::cout << "========++ NUM CELLS " << kv.second->get_num_cells() << std::endl;
        kv.second->prepare_dataset();
    }
}

int ReportingLib::record_data(double timestep, int ncells, int* cellids, const std::string& report_name) {

    ReportMap::iterator reportFinder = m_reports.find(report_name);
    if (reportFinder == m_reports.end()) {
        std::cout << "Report '" << report_name << "' doesn't exist!" << std::endl;
        return -1;
    }
    //m_logger->info("Recording data for report {}, timestep {}", report_name, timestep);
    m_reports[report_name]->recData(timestep, ncells, cellids);
    return 0;
}

int ReportingLib::end_iteration(double timestep) {

    for (auto& kv : m_reports) {
        kv.second->end_iteration(timestep);
    }
    return 0;
}

int ReportingLib::flush(double time) {

    for (auto &kv : m_reports) {
        kv.second->flush(time);
    }
    return 0;
}

int ReportingLib::set_max_buffer_size(const std::string& report_name, size_t buf_size) {

    ReportMap::iterator reportFinder = m_reports.find(report_name);
    if (reportFinder == m_reports.end()) {
        std::cout << "Report '" << report_name << "' doesn't exist!" << std::endl;
        return -1;
    }
    m_reports[report_name]->set_max_buffer_size(buf_size);
    return 0;
}

int ReportingLib::set_max_buffer_size(size_t buf_size) {

    for (auto &kv : m_reports) {
        kv.second->set_max_buffer_size(buf_size);
    }
    return 0;
}