#include <iostream>

#include "element_report.hpp"

ElementReport::ElementReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t ElementReport::get_total_elements() const noexcept {
    size_t total = 0;
    for(auto& kv: *m_nodes) {
        total += kv.second->get_num_elements();
    }
    return total;
}

