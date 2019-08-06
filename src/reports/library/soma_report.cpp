#include <iostream>
#include <exception>

#include "soma_report.hpp"

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SomaReport::get_total_elements() const {
    // Every node has only 1 element on a soma report
    return m_nodes->size();
}

bool SomaReport::check_add_variable(uint64_t node_id) {
    if (m_nodes->find(node_id) != m_nodes->end()) {
        if((*m_nodes)[node_id].get_num_elements() == 0) {
            return true;
        } else {
            throw std::runtime_error("ERROR: Soma report nodes can only have 1 element");
        }
    } else {
        throw std::runtime_error("ERROR: Searching this node: " + std::to_string(node_id));
    }
}

int SomaReport::add_variable(uint64_t node_id, double* element_value, uint32_t compartment_id) {

    if (check_add_variable(node_id)) {
        (*m_nodes)[node_id].add_element(element_value, compartment_id);
    }
    return 0;
}
