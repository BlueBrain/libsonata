#pragma once

#include <vector>

class Node {
  private:
    uint64_t m_gid;
    uint64_t m_vgid;
    uint32_t m_compartment_id;

    std::vector<uint32_t > m_compartment_ids;
    std::vector<double*> m_compartments;
    std::vector<double*> m_spikes;

public:
    Node(uint64_t gid, uint64_t vgid);
    Node() : m_gid(0), m_vgid(0) {};
    ~Node() = default;

    int fill_data(double* data, bool node_to_be_recorded);

    void add_compartment(double* compartment_value);
    void add_spike(double* spike_timestamp);
    size_t get_num_compartments () const { return m_compartments.size(); }
    size_t get_num_spikes () const { return m_spikes.size(); }

    const std::vector<uint32_t>& get_compartment_ids() const { return m_compartment_ids; }
    const std::vector<double*>& get_spike_timestamps() const { return m_spikes; }
};