#include <algorithm>

#include "node.hpp"

namespace bbp {
namespace sonata {

Node::Node(uint64_t gid)
    : m_gid(gid)
{}

void Node::add_element(double* element_value, uint32_t element_id) {
    m_elements.push_back(element_value);
    m_element_ids.push_back(element_id);
}

void Node::fill_data(std::vector<double>::iterator it) {
    std::transform(m_elements.begin(), m_elements.end(), it, [](auto elem){ return *elem; });
}

void Node::refresh_pointers(std::function<double*(double*)> refresh_function) {
    std::transform(m_elements.begin(), m_elements.end(), m_elements.begin(), refresh_function);
}

}
} // namespace bbp::sonata
