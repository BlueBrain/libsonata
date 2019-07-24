#include <iostream>

#include "reportinglib.hpp"

double ReportingLib::m_atomicStep = 1e-8;
#ifdef HAVE_MPI
MPI_Comm ReportingLib::m_allCells = MPI_COMM_WORLD;
int ReportingLib::m_rank = 0;
#endif

ReportingLib::ReportingLib(): m_numReports(0) {

}

ReportingLib::~ReportingLib() {
    clear();
}

void ReportingLib::clear() {

    for (auto& kv: m_reports) {
        std::cout << "Deleting " << kv.first << std::endl;
    }
    m_reports.clear();
}

bool ReportingLib::is_empty() {
    return (!m_numReports);
}

int ReportingLib::get_num_reports() {
    return m_numReports;
}

Report::Kind ReportingLib::string_to_enum(const std::string& kind) {

    if(kind == "compartment") {
        return Report::Kind::COMPARTMENT;
    } else if (kind == "soma") {
        return Report::Kind::SOMA;
    } else if (kind == "spike") {
        return Report::Kind::SPIKE;
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
        Report::Kind kind_report = string_to_enum(kind);
        // new report
        report = Report::createReport(report_name, tstart, tend, dt, kind_report);
        // Check if kind doesnt exist
        if(report) {
            m_reports[report_name] = report;
            m_numReports++;
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

#ifdef HAVE_MPI
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
#endif
}

void ReportingLib::share_and_prepare() {

#ifdef HAVE_MPI
    int rank, num_nodes;
    MPI_Comm_rank(ReportingLib::m_allCells, &rank);
    MPI_Comm_size(ReportingLib::m_allCells, &num_nodes);

    std::cout << "Hello from rank " << rank << " , size: " << num_nodes << std::endl;
    m_rank = rank;

    // Split reports into different ranks?

    // Create communicator groups ?

#endif
    // remove reports without cells
    for(auto& kv: m_reports) {
        if(kv.second->is_empty()){
            m_reports.erase(kv.first);
        }
    }

    // Allocate buffers
    for (auto& kv : m_reports) {
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

    for (auto& kv : m_reports) {
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

    for (auto& kv : m_reports) {
        kv.second->set_max_buffer_size(buf_size);
    }
    return 0;
}