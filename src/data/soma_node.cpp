#include <stdexcept>

#include "soma_node.h"

namespace bbp {
namespace sonata {

SomaNode::SomaNode(uint64_t node_id)
    : Node(node_id) {}

void SomaNode::add_element(double* element_value, uint32_t element_id) {
    if (!elements_.empty()) {
        throw std::runtime_error("ERROR: Soma report nodes can only have 1 element");
    }
    elements_.push_back(element_value);
    element_ids_.push_back(element_id);
}

}  // namespace sonata
}  // namespace bbp
