#include <vector>
#include <fstream>
#include <hdf5.h>

#include "io_writer.hpp"
#include "h5typemap.hpp"

class HDF5Writer: public IoWriter {
  private:
    hid_t m_file;
    hid_t m_dataset;

    hid_t m_collective_list;
    hid_t m_independent_list;

    hsize_t m_offset[2];

  public:
    HDF5Writer(const std::string& report_name);
    ~HDF5Writer() = default;

    void configure_group(const std::string& group_name);
    void configure_attribute(const std::string& group_name, const std::string& attribute_name, const std::string& attribute_value);
    void configure_dataset(const std::string& dataset_name, int total_steps, int total_elements);
    void write(double* buffer, int steps_to_write, int total_steps, int total_elements) override;
    void write(const std::string& dataset_name, const std::vector<int>& buffer) override;
    void write(const std::string& dataset_name, const std::vector<uint32_t>& buffer) override;
    void write(const std::string& dataset_name, const std::vector<uint64_t>& buffer) override;
    void write(const std::string& dataset_name, const std::vector<float>& buffer) override;
    void write(const std::string& dataset_name, const std::vector<double>& buffer) override;

    template <typename T>
    void write_any(const std::string& name, const std::vector<T>& buffer);

    void close() override;
};