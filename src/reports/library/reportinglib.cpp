#include <iostream>

#include <reports/utils/logger.hpp>
#include "implementation_interface.hpp"
#include "reportinglib.hpp"
#include "soma_report.hpp"
#include "element_report.hpp"

double ReportingLib::m_atomic_step = 1e-8;
double ReportingLib::m_min_steps_to_record = 0.0;
bool ReportingLib::first_report = true;
int ReportingLib::m_rank = 0;
#ifdef HAVE_MPI
MPI_Comm ReportingLib::m_has_nodes = MPI_COMM_WORLD;
ReportingLib::communicators_t ReportingLib::m_communicators;
#endif

void ReportingLib::clear() {
    for (auto& kv: m_reports) {
        logger->debug("Deleting report: {} from rank {}", kv.first, ReportingLib::m_rank);
    }
    m_reports.clear();
}

bool ReportingLib::is_empty() {
    return m_reports.empty();
}

std::shared_ptr<Report> ReportingLib::create_report(const std::string& name,
        const std::string& kind, double tstart, double tend, double dt) {
    if (kind == "compartment") {
        m_reports.emplace(name, std::make_shared<ElementReport>(name, tstart, tend, dt));
    } else if (kind == "soma") {
        m_reports.emplace(name, std::make_shared<SomaReport>(name, tstart, tend, dt));
    } else {
        throw std::runtime_error("Kind '" + kind + "' doesn't exist!");
    }
    logger->debug("Creating report {} type {} tstart {} and tstop {} from rank {}", name, kind, tstart, tend, m_rank);

    return get_report(name);
}

std::shared_ptr<Report> ReportingLib::get_report(const std::string& name) const {
    return m_reports.at(name);
}

bool ReportingLib::report_exists(const std::string& name) const {
    return m_reports.find(name) != m_reports.end();
}

void ReportingLib::make_global_communicator() {
    std::vector<std::string> report_names;
    report_names.reserve(m_reports.size());
    for(auto& kv: m_reports) {
        report_names.push_back(kv.first);
    }

    // Split reports into different ranks
    // Create communicator groups
    m_rank = Implementation::init(report_names);
    if(m_rank == 0) {
        logger->info("Initializing communicators and preparing SONATA datasets");
    }
}

void ReportingLib::prepare_datasets() {
    for(auto& kv: m_reports) {
        // remove reports without nodes
        if(kv.second->is_empty()){
            m_reports.erase(kv.first);
            continue;
        }

        logger->debug("Preparing datasets of report {} from rank {} with {} NODES", kv.first, m_rank, kv.second->get_num_nodes());
        kv.second->prepare_dataset();
    }
}

void ReportingLib::write_spikes(const std::vector<double>& spike_timestamps, const std::vector<int>& spike_node_ids) {
    SonataData spike_data ("spikes", spike_timestamps, spike_node_ids);
    spike_data.write_spikes_header();
    spike_data.close();
}

