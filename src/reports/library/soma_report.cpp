#include <iostream>
#include <exception>

#include "soma_report.hpp"

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SomaReport::get_total_elements() const noexcept {
    // Every node has only 1 element on a soma report
    return m_nodes->size();
}

bool SomaReport::check_add_variable(uint64_t node_id) {
    if (!node_exists(node_id)) {
        throw std::runtime_error("ERROR: Searching this node: " + std::to_string(node_id));
    }

    if(get_node(node_id).get_num_elements() > 0) {
        throw std::runtime_error("ERROR: Soma report nodes can only have 1 element");
    }

    return true;
}

