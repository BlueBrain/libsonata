#pragma once

#include <vector>

class Cell {
  private:
    int m_gid;
    int m_vgid;
    int m_compartment_id;

    std::vector<uint32_t > m_compartment_ids;
    std::vector<double*> m_compartments;
    std::vector<double*> m_spikes;

public:
    Cell(int gid, int vgid);
    Cell() : m_gid(0), m_vgid(0) {};
    ~Cell() = default;

    int fill_data(float* data, bool cell_to_be_recorded);

    void add_compartment(double* compartment_value);
    void add_spike(double* spike_timestamp);
    size_t get_num_compartments();
    size_t get_num_spikes();

    const std::vector<uint32_t>& get_compartment_ids() { return m_compartment_ids; }
    const std::vector<double*>& get_spike_timestamps() { return m_spikes; }
};