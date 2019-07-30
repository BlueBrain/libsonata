#include <iostream>

#include "compartment_report.hpp"

CompartmentReport::CompartmentReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t CompartmentReport::get_total_compartments() const {

    size_t total = 0;
    for(auto& kv: *m_nodes) {
        total += kv.second.get_num_compartments();
    }
    return total;
}

int CompartmentReport::add_variable(uint64_t node_id, double* element_value) {

    auto node_finder = m_nodes->find(node_id);
    if (node_finder != m_nodes->end()) {
        node_finder->second.add_compartment(element_value);
    } else {
        std::cerr << "ERROR: Searching this node: " << node_id << std::endl;
        return -1;
    }
    return 0;
}
