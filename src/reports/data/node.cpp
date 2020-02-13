#include <algorithm>

#include "node.h"

namespace bbp {
namespace sonata {

Node::Node(uint64_t node_id)
    : node_id_(node_id) {}

void Node::add_element(double* element_value, uint32_t element_id) {
    elements_.push_back(element_value);
    element_ids_.push_back(element_id);
}

void Node::fill_data(std::vector<double>::iterator it) {
    std::transform(elements_.begin(), elements_.end(), it, [](auto elem) { return *elem; });
}

void Node::refresh_pointers(std::function<double*(double*)> refresh_function) {
    std::transform(elements_.begin(), elements_.end(), elements_.begin(), refresh_function);
}

}  // namespace sonata
}  // namespace bbp
