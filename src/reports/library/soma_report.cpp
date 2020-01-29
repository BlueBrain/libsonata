#include <exception>
#include <iostream>

#include "soma_report.hpp"
#include <reports/data/soma_node.hpp>

namespace bbp {
namespace sonata {

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
    : Report(report_name, tstart, tend, dt) {}

void SomaReport::add_node(uint64_t node_id) {
    if (node_exists(node_id)) {
        throw std::runtime_error("Warning: attempted to add node " + std::to_string(node_id) +
                                 " to the target multiple times on same report. Ignoring.");
    }
    m_nodes->emplace(node_id, std::make_shared<SomaNode>(node_id));
}

size_t SomaReport::get_total_elements() const noexcept {
    // Every node has only 1 element on a soma report
    return m_nodes->size();
}

}  // namespace sonata
}  // namespace bbp
