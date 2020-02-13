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
    void configure_attribute(const std::string& group_name,
                             const std::string& attribute_name,
                             const std::string& attribute_value);
    void configure_dataset(const std::string& dataset_name, int total_steps, int total_elements);
    void write(const std::vector<double>& buffer, int steps_to_write, int total_elements);
    void write(const std::string& dataset_name, const std::vector<uint32_t>& buffer);
    void write(const std::string& dataset_name, const std::vector<uint64_t>& buffer);
    void write(const std::string& dataset_name, const std::vector<float>& buffer);
    void write(const std::string& dataset_name, const std::vector<double>& buffer);
    template <typename T>
    void write_any(const std::string& name, const std::vector<T>& buffer);
    void close();

  private:
    std::string report_name_;

    hid_t file_;
    hid_t dataset_;
    hid_t collective_list_;
    hid_t independent_list_;
    std::array<hsize_t, 2> offset_;
};

}  // namespace sonata
}  // namespace bbp
