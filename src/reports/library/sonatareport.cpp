#include <iostream>

#include "element_report.hpp"
#include "implementation_interface.hpp"
#include "soma_report.hpp"
#include "sonatareport.hpp"
#include <reports/utils/logger.hpp>

namespace bbp {
namespace sonata {

double SonataReport::m_atomic_step = 1e-8;
double SonataReport::m_min_steps_to_record = 0.0;
bool SonataReport::first_report = true;
int SonataReport::m_rank = 0;
#ifdef HAVE_MPI
MPI_Comm SonataReport::m_has_nodes = MPI_COMM_WORLD;
SonataReport::communicators_t SonataReport::m_communicators;
#endif

void SonataReport::clear() {
    for (auto& kv : m_reports) {
        logger->debug("Deleting report: {} from rank {}", kv.first, SonataReport::m_rank);
    }
    m_reports.clear();
}

bool SonataReport::is_empty() {
    return m_reports.empty();
}

std::shared_ptr<Report> SonataReport::create_report(
    const std::string& name, const std::string& kind, double tstart, double tend, double dt) {
    if (kind == "compartment") {
        m_reports.emplace(name, std::make_shared<ElementReport>(name, tstart, tend, dt));
    } else if (kind == "soma") {
        m_reports.emplace(name, std::make_shared<SomaReport>(name, tstart, tend, dt));
    } else {
        throw std::runtime_error("Kind '" + kind + "' doesn't exist!");
    }
    logger->debug("Creating report {} type {} tstart {} and tstop {} from rank {}",
                  name,
                  kind,
                  tstart,
                  tend,
                  m_rank);

    return get_report(name);
}

std::shared_ptr<Report> SonataReport::get_report(const std::string& name) const {
    return m_reports.at(name);
}

bool SonataReport::report_exists(const std::string& name) const {
    return m_reports.find(name) != m_reports.end();
}

void SonataReport::create_communicators() {
    std::vector<std::string> report_names;
    report_names.reserve(m_reports.size());
    for (auto iter = m_reports.begin(); iter != m_reports.end();) {
        if (iter->second->is_empty()) {
            // Remove reports without nodes
            iter = m_reports.erase(iter);
        } else {
            report_names.push_back(iter->first);
            ++iter;
        }
    }
    // Create communicator groups
    m_rank = Implementation::init(report_names);
    if (m_rank == 0 && !is_empty()) {
        logger->info("Initializing communicators and preparing SONATA datasets");
    }
}

void SonataReport::prepare_datasets() {
    for (auto& kv : m_reports) {
        logger->debug("Preparing datasets of report {} from rank {} with {} NODES",
                      kv.first,
                      m_rank,
                      kv.second->get_num_nodes());
        kv.second->prepare_dataset();
    }
}

void SonataReport::write_spikes(const std::vector<double>& spike_timestamps,
                                const std::vector<int>& spike_node_ids) {
    SonataData spike_data("spikes", spike_timestamps, spike_node_ids);
    spike_data.write_spikes_header();
    spike_data.close();
}

}  // namespace sonata
}  // namespace bbp
