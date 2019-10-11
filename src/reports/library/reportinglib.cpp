#include <iostream>

#include <reports/utils/logger.hpp>
#include "implementation_interface.hpp"
#include "reportinglib.hpp"
#include "soma_report.hpp"
#include "element_report.hpp"
#include "spike_report.hpp"

double ReportingLib::m_atomic_step = 1e-8;
double ReportingLib::m_mindelay = 0.0;
bool ReportingLib::first_report = true;
#ifdef HAVE_MPI
MPI_Comm ReportingLib::m_has_nodes = MPI_COMM_WORLD;
ReportingLib::communicators_t ReportingLib::m_communicators;
int ReportingLib::m_rank = 0;
#endif

ReportingLib::ReportingLib(): m_num_reports(0) {
}

ReportingLib::~ReportingLib() {
    clear();
}

void ReportingLib::clear() {
    for (auto& kv: m_reports) {
        logger->debug("Deleting report: {} from rank {}", kv.first, ReportingLib::m_rank);
    }
    m_reports.clear();
}

bool ReportingLib::is_empty() {
    return (!m_num_reports);
}

int ReportingLib::get_num_reports() const {
    return m_num_reports;
}

int ReportingLib::add_report(const std::string& report_name, uint64_t node_id, uint64_t gid, uint64_t vgid,
                             double tstart, double tend, double dt, const std::string& kind) {
    try {
        std::shared_ptr <Report> report;
        // check if this is the first time a Report with the given name is referenced
        if (m_reports.find(report_name) != m_reports.end()) {
            report = m_reports[report_name];
        } else {
            // new report
            if (kind == "compartment") {
                report = std::make_shared<ElementReport>(report_name, tstart, tend, dt);
            } else if (kind == "soma") {
                report = std::make_shared<SomaReport>(report_name, tstart, tend, dt);
            } else if (kind == "spike") {
                report = std::make_shared<SpikeReport>(report_name, tstart, tend, dt);
            } else {
                throw std::runtime_error("Kind " + kind + " doesn't exist!");
            }
            // Check if kind doesnt exist
            if (report) {
                logger->trace("Creating report {} type {} tstart{} and tstop {} from rank {}", report_name, kind, tstart, tend, m_rank);
                m_reports[report_name] = report;
                m_num_reports++;
            }
        }
        if (report) {
            report->add_node(node_id, gid, vgid);
        }
    } catch (const std::exception& ex) {
        logger->error(ex.what());
    }
    return 0;
}

int ReportingLib::add_variable(const std::string& report_name, uint64_t node_id, double* voltage, uint32_t element_id) {
    try {
        if (m_reports.find(report_name) != m_reports.end()) {
            return m_reports[report_name]->add_variable(node_id, voltage, element_id);
        }
    } catch (const std::exception& ex) {
        logger->error(ex.what());
    }
    return 0;
}

void ReportingLib::make_global_communicator() {
    std::vector<std::string> report_names;
    report_names.reserve(m_num_reports);
    for(auto& kv: m_reports) {
        report_names.push_back(kv.first);
    }

    // Split reports into different ranks
    // Create communicator groups
    m_rank = Implementation::init(report_names);
    if(m_rank == 0) {
        logger->info("Initializing communicators and preparing datasets with mindelay={}", m_mindelay);
    }
}

void ReportingLib::prepare_datasets() {
    // remove reports without nodes
    for(auto& kv: m_reports) {
        if(kv.second->is_empty()){
            m_reports.erase(kv.first);
        }
    }

    // Allocate buffers
    for (auto& kv : m_reports) {
        logger->info("Preparing datasets of report {} from rank {} with {} NODES", kv.first, m_rank, kv.second->get_num_nodes());
        kv.second->prepare_dataset();
    }
}

int ReportingLib::record_nodes_data(double step, const std::vector<uint64_t>& node_ids, const std::string& report_name) {
    if (m_reports.find(report_name) == m_reports.end()) {
        logger->warn("Report {} doesn't exist!", report_name);
        return -1;
    }
    m_reports[report_name]->record_data(step, node_ids);
    return 0;
}

int ReportingLib::record_data(double step) {
    for (auto& kv : m_reports) {
        kv.second->record_data(step);
    }
    return 0;
}

int ReportingLib::end_iteration(double timestep) {
    for (auto& kv : m_reports) {
        kv.second->end_iteration(timestep);
    }
    return 0;
}

void ReportingLib::refresh_pointers(refresh_function_t refresh_function) {
    for (auto& kv : m_reports) {
        kv.second->refresh_pointers(refresh_function);
    }
}

int ReportingLib::flush(double time) {
    for (auto& kv : m_reports) {
        kv.second->flush(time);
    }
    return 0;
}

int ReportingLib::set_max_buffer_size(const std::string& report_name, size_t buffer_size) {
    if (m_reports.find(report_name) == m_reports.end()) {
        logger->warn("Report {} doesn't exist!", report_name);
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
