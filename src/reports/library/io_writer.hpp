#pragma once
#include <vector>
#include <memory>

class IoWriter {
  protected:
    std::string m_reportName;

  public:
    enum class Kind {HDF5, ASCII, MPIIO};

    IoWriter(const std::string& report_name);
    virtual ~IoWriter() = default;

    static std::unique_ptr<IoWriter> create_IoWriter(IoWriter::Kind kind, const std::string& report_name);

    // Not implemented by default, override by hdf5
    virtual void configure_group(const std::string& group_name) {}
    virtual void configure_attribute(const std::string& group_name, const std::string& attribute_name) {}
    virtual void configure_dataset(const std::string& dataset_name, int total_steps, int total_compartments) {}
    virtual void write(const std::string& name, float* buffer, int steps_to_write, int total_steps, int total_compartments) = 0;
    virtual void write(const std::string& name, const std::vector<int>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<uint32_t>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<uint64_t>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<float>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<double>& buffer) = 0;

    virtual void close() = 0;
};