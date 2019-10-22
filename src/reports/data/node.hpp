#pragma once

#include <vector>

typedef double* (*refresh_function_t)(double*);

class Node {
  private:
    uint64_t m_gid;
    uint64_t m_vgid;
    uint32_t m_element_id;

    std::vector<uint32_t > m_element_ids;
    std::vector<double*> m_elements;

public:
    Node(uint64_t gid, uint64_t vgid);
    Node() : m_gid(0), m_vgid(0) {};
    ~Node() = default;

    int fill_data(double* data, bool node_to_be_recorded);
    void refresh_pointers(refresh_function_t refresh_function);

    void add_element(double* element_value, uint32_t element_id);
    size_t get_num_elements () const { return m_elements.size(); }
    
    const std::vector<uint32_t>& get_element_ids() const { return m_element_ids; }
};
