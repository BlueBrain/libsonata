#include <stdexcept>

#include "soma_node.hpp"

namespace bbp {
namespace sonata {

SomaNode::SomaNode(uint64_t gid)
    : Node(gid) {}

void SomaNode::add_element(double* element_value, uint32_t element_id) {
    if (!m_elements.empty()) {
        throw std::runtime_error("ERROR: Soma report nodes can only have 1 element");
    }
    m_elements.push_back(element_value);
    m_element_ids.push_back(element_id);
}

}  // namespace sonata
}  // namespace bbp
