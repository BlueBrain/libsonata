#include <iostream>

#include "element_report.hpp"

ElementReport::ElementReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t ElementReport::get_total_elements() const {

    size_t total = 0;
    for(auto& kv: *m_nodes) {
        total += kv.second.get_num_elements();
    }
    return total;
}

int ElementReport::add_variable(uint64_t node_id, double* element_value) {

    if (check_add_variable(node_id)) {
        (*m_nodes)[node_id].add_element(element_value);
    }
    return 0;
}
