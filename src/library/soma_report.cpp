#include <exception>
#include <iostream>

#include "../data/soma_node.h"
#include "soma_report.h"

namespace bbp {
namespace sonata {

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
    : Report(report_name, tstart, tend, dt) {}

void SomaReport::add_node(const std::string& population_name, uint64_t node_id) {
    if (population_exists(population_name)) {
        if (node_exists(population_name, node_id)) {
            throw std::runtime_error("Warning: attempted to add node " + std::to_string(node_id) +
                                     " to the target multiple times on same report. Ignoring.");
        }
        std::shared_ptr<nodes_t> nodes = populations_->at(population_name);
        nodes->emplace(node_id, std::make_shared<SomaNode>(node_id));
    } else {
        // node is new insert it into the map
        std::shared_ptr<nodes_t> nodes = std::make_shared<nodes_t>();
        nodes->emplace(node_id, std::make_shared<SomaNode>(node_id));
        populations_->emplace(population_name, nodes);
    }
}

size_t SomaReport::get_total_elements(const std::string& population_name) const {
    // Every node has only 1 element on a soma report
    return populations_->at(population_name)->size();
}

}  // namespace sonata
}  // namespace bbp
