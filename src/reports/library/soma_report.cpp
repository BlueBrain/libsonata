#include <iostream>

#include "soma_report.hpp"

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SomaReport::get_total_compartments() const {
    // Every node has only 1 compartment on a soma report
    return m_nodes->size();
}

int SomaReport::add_variable(uint64_t node_id, double* element_value) {

    /*TODO:
     * auto node = check_node_with_compartment(id);
     * node->add_compartment(pointer);
     */
    auto node_finder = m_nodes->find(node_id);
    if (node_finder != m_nodes->end()) {
        if(node_finder->second.get_num_compartments() == 0) {
            node_finder->second.add_compartment(element_value);
        } else {
            std::cerr << "ERROR: Soma report nodes can only have 1 element" << std::endl;
            return -1;
        }
    } else {
        std::cerr << "ERROR: Searching this node: " << node_id << std::endl;
        return -1;
    }
    return 0;
}
