#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace bbp {
namespace sonata {

class Node {
public:
    Node(uint64_t gid);
    virtual ~Node() = default;

    void fill_data(std::vector<double>::iterator it);
    void refresh_pointers(std::function<double*(double*)> refresh_function);
    virtual void add_element(double* element_value, uint32_t element_id);

    uint64_t get_gid() const noexcept { return m_gid; }
    virtual size_t get_num_elements() const noexcept { return m_elements.size(); }
    const std::vector<uint32_t>& get_element_ids() const noexcept { return m_element_ids; }

  private:
    uint64_t m_gid;

  protected:
    std::vector<uint32_t> m_element_ids;
    std::vector<double*> m_elements;
};

}
} // namespace bbp::sonata
