#include <iostream>

#include "node.hpp"

Node::Node(uint64_t gid, uint64_t vgid): m_gid(gid), m_vgid(vgid) {
    // element id would be gid+element_id.
    //  i.e: for gid 3 and element 42 -> 3042
    // TODO: what should be the id format?
    m_element_id = gid*1000;
}

void Node::add_element(double* element_value) {
    m_elements.push_back(element_value);
    m_element_ids.push_back(m_element_id);
    m_element_id++;
}

void Node::add_spike(double* spike_timestamp) {
    m_spikes.push_back(spike_timestamp);
}

int Node::fill_data(double* data, bool node_to_be_recorded) {
    // Copy data from node data structures to buffer
    int reporting_total = 0;
    if(node_to_be_recorded) {
        for (auto &elem: m_elements) {
            data[reporting_total++] = *elem;
        }
    } else {
        // Set all element values to default values
        int num_elements = m_elements.size();
        std::fill(data, data + num_elements, 0.0);
        reporting_total = num_elements;
    }
    return reporting_total;
}