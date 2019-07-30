#include <iostream>

#include "reportinglib.hpp"
#include "implementation_interface.hpp"

double ReportingLib::m_atomic_step = 1e-8;
bool ReportingLib::first_report = true;
#ifdef HAVE_MPI
MPI_Comm ReportingLib::m_has_nodes = MPI_COMM_WORLD;
int ReportingLib::m_rank = 0;
#endif

ReportingLib::ReportingLib(): m_num_reports(0) {
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
    return (!m_num_reports);
}

int ReportingLib::get_num_reports() const {
    return m_num_reports;
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

int ReportingLib::add_report(const std::string& report_name, uint64_t node_id, uint64_t gid, uint64_t vgid,
                             double tstart, double tend, double dt, const std::string& kind) {

    // check if this is the first time a Report with the given name is referenced
    auto report_finder = m_reports.find(report_name);

    std::shared_ptr<Report> report;
    if (report_finder != m_reports.end()) {
        report = report_finder->second;
        std::cout << "Report '" << report_name << "' found!" << std::endl;
    } else {
        Report::Kind kind_report = string_to_enum(kind);
        // new report
        // TODO: remove factory and instantiate here
        report = Report::create_report(report_name, tstart, tend, dt, kind_report);
        // Check if kind doesnt exist
        if(report) {
            m_reports[report_name] = report;
            m_num_reports++;
        }
    }

    if (report) {
        report->add_node(node_id, gid, vgid);
    }
    return 0;
}

int ReportingLib::add_variable(const std::string& report_name, uint64_t node_id, double* voltage) {

    auto report_finder = m_reports.find(report_name);
    if (report_finder != m_reports.end()) {
        return report_finder->second->add_variable(node_id, voltage);
    }

    return 0;
}

void ReportingLib::make_global_communicator() {

    Implementation::init_comm(m_num_reports);
}

void ReportingLib::share_and_prepare() {

    // Split reports into different ranks
    // Create communicator groups
    m_rank = Implementation::init();

    // remove reports without nodes
    for(auto& kv: m_reports) {
        if(kv.second->is_empty()){
            m_reports.erase(kv.first);
        }
    }

    // Allocate buffers
    for (auto& kv : m_reports) {
        std::cout << "========++ NUM NODES " << kv.second->get_num_nodes() << std::endl;
        kv.second->prepare_dataset();
    }
}

int ReportingLib::record_data(double timestep, const std::vector<uint64_t>& node_ids, const std::string& report_name) {

    if (m_reports.find(report_name) == m_reports.end()) {
        std::cout << "Report '" << report_name << "' doesn't exist!" << std::endl;
        return -1;
    }
    m_reports[report_name]->record_data(timestep, node_ids);
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

int ReportingLib::set_max_buffer_size(const std::string& report_name, size_t buffer_size) {

    if (m_reports.find(report_name) == m_reports.end()) {
        std::cout << "Report '" << report_name << "' doesn't exist!" << std::endl;
        return -1;
    }
    m_reports[report_name]->set_max_buffer_size(buffer_size);
    return 0;
}

int ReportingLib::set_max_buffer_size(size_t buffer_size) {

    for (auto& kv : m_reports) {
        kv.second->set_max_buffer_size(buffer_size);
    }
    return 0;
}