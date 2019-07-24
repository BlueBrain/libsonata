#include <iostream>

#include "cell.hpp"

Cell::Cell(int gid, int vgid): m_gid(gid), m_vgid(vgid) {
    // Compartment id would be gid+compartment_id.
    //  i.e: for gid 3 and compartment 42 -> 3042
    // TODO: what should be the id format?
    m_compartment_id = gid*1000;
}

size_t Cell::get_num_compartments() {
    return m_compartments.size();
}

size_t Cell::get_num_spikes() {
    return m_spikes.size();
}

void Cell::add_compartment(double* compartment_value) {
    m_compartments.push_back(compartment_value);
    m_compartment_ids.push_back(m_compartment_id);
    m_compartment_id++;
}

void Cell::add_spike(double* spike_timestamp) {
    m_spikes.push_back(spike_timestamp);
}

int Cell::fill_data(float* data, bool cell_to_be_recorded) {
    // Copy data from cell data structures to buffer
    int reportingTotal = 0;
    if(cell_to_be_recorded) {
        for (auto &elem: m_compartments) {
            data[reportingTotal++] = static_cast<float>(*elem);
        }
    } else {
        // Set all compartment values to default values
        int num_compartments = m_compartments.size();
        std::fill(data, data + num_compartments, 0.0);
        reportingTotal = num_compartments;
    }
    return reportingTotal;
}