#pragma once

#include <cstdint>
#include <vector>

typedef double* (*refresh_function_t)(double*);

class Node {
public:
    Node(uint64_t gid);

    size_t fill_data(double* data);
    void refresh_pointers(refresh_function_t refresh_function);
    void add_element(double* element_value, uint32_t element_id);

    uint64_t get_gid() const noexcept { return m_gid; }
    size_t get_num_elements() const noexcept { return m_elements.size(); }
    const std::vector<uint32_t>& get_element_ids() const noexcept { return m_element_ids; }

  private:
    uint64_t m_gid;

    std::vector<uint32_t> m_element_ids;
    std::vector<double*> m_elements;
};

