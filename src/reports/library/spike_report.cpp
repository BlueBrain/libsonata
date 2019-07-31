#include <iostream>

#include "spike_report.hpp"

SpikeReport::SpikeReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SpikeReport::get_total_elements() const {
    // Spike report doesnt have elements
    return 0;
}

int SpikeReport::add_variable(uint64_t node_id, double* spike_value) {

    if (check_add_variable(node_id)) {
        (*m_nodes)[node_id].add_spike(spike_value);
    }
    return 0;
}