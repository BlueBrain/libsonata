#include <iostream>

#include "node.hpp"

Node::Node(uint64_t gid, uint64_t vgid): m_gid(gid), m_vgid(vgid) {
    // Compartment id would be gid+compartment_id.
    //  i.e: for gid 3 and compartment 42 -> 3042
    // TODO: what should be the id format?
    m_compartment_id = gid*1000;
}

void Node::add_compartment(double* compartment_value) {
    m_compartments.push_back(compartment_value);
    m_compartment_ids.push_back(m_compartment_id);
    m_compartment_id++;
}

void Node::add_spike(double* spike_timestamp) {
    m_spikes.push_back(spike_timestamp);
}

int Node::fill_data(double* data, bool node_to_be_recorded) {
    // Copy data from node data structures to buffer
    int reporting_total = 0;
    if(node_to_be_recorded) {
        for (auto &elem: m_compartments) {
            data[reporting_total++] = *elem;
        }
    } else {
        // Set all compartment values to default values
        int num_compartments = m_compartments.size();
        std::fill(data, data + num_compartments, 0.0);
        reporting_total = num_compartments;
    }
    return reporting_total;
}