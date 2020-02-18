#pragma once
#include <cstdint>
#include <functional>
#include <vector>

namespace bbp {
namespace sonata {

class Node
{
  public:
    explicit Node(uint64_t node_id);

    void fill_data(std::vector<double>::iterator it);
    void refresh_pointers(std::function<double*(double*)> refresh_function);
    virtual void add_element(double* element_value, uint32_t element_id);

    uint64_t get_node_id() const noexcept {
        return node_id_;
    }
    virtual size_t get_num_elements() const noexcept {
        return elements_.size();
    }
    const std::vector<uint32_t>& get_element_ids() const noexcept {
        return element_ids_;
    }

  private:
    uint64_t node_id_;

  protected:
    std::vector<uint32_t> element_ids_;
    std::vector<double*> elements_;
};

}  // namespace sonata
}  // namespace bbp
