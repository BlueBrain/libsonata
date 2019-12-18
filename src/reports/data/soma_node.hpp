#pragma once

#include "node.hpp"

namespace bbp {
namespace sonata {

class SomaNode: public Node
{
  public:
    SomaNode(uint64_t gid);

    void add_element(double* element_value, uint32_t element_id) override;
    size_t get_num_elements() const noexcept override {
        return m_elements.empty() ? 0 : 1;
    };
};

}  // namespace sonata
}  // namespace bbp
