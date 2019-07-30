#include <iostream>

#include "spike_report.hpp"

SpikeReport::SpikeReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SpikeReport::get_total_compartments() const {
    // Spike report doesnt have compartments
    return 0;
}

int SpikeReport::add_variable(uint64_t node_id, double* spike_value) {

    auto node_finder = m_nodes->find(node_id);
    if (node_finder != m_nodes->end()) {
        node_finder->second.add_spike(spike_value);
    } else {
        std::cerr << "ERROR: Searching this node: " << node_id << std::endl;
        return -1;
    }
    return 0;
}