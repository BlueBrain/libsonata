#pragma once
#include <array>
#include <fstream>
#include <hdf5.h>
#include <vector>

#include "h5typemap.hpp"

namespace bbp {
namespace sonata {

class HDF5Writer
{
  public:
    explicit HDF5Writer(const std::string& report_name);
    void configure_group(const std::string& group_name);
    void configure_attribute(const std::string& dataset_name,
                             const std::string& attribute_name,
                             const std::string& attribute_value);
    void configure_enum_attribute(const std::string& group_name,
                                  const std::string& attribute_name,
                                  const std::string& attribute_value);
    void configure_dataset(const std::string& dataset_name,
                           uint32_t total_steps,
                           uint32_t total_elements);
    void write_2D(const std::vector<float>& buffer,
                  uint32_t steps_to_write,
                  uint32_t total_elements);
    template <typename T>
    void write(const std::string& name, const std::vector<T>& buffer);
    void write_time(const std::string& dataset_name, const std::array<double, 3>& buffer);
    void close();

  private:
    std::string report_name_;

    hid_t file_ = 0;
    hid_t dataset_ = 0;
    hid_t collective_list_ = 0;
    hid_t independent_list_ = 0;
    hid_t spikes_attr_type_ = 0;
    std::array<hsize_t, 2> offset_ = {0, 0};
};

}  // namespace sonata
}  // namespace bbp
