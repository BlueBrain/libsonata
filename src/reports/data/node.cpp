#include <iostream>

#include "node.hpp"

Node::Node(uint64_t gid)
    : m_gid(gid)
{}

uint64_t Node::get_gid() const {
    return m_gid;
}

void Node::add_element(double* element_value, uint32_t element_id) {
    m_elements.push_back(element_value);
    m_element_ids.push_back(element_id);
}

size_t Node::fill_data(double* data) {
    // Copy data from node data structures to buffer
    for (auto &elem: m_elements) {
        0[data++] = *elem;
    }

    return get_num_elements();
}

void Node::refresh_pointers(refresh_function_t refresh_function) {
    for (auto &elem: m_elements) {
        elem = refresh_function(elem);
    }
}

