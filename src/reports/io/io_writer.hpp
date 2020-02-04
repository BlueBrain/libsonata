#pragma once
#include <memory>
#include <vector>

class IoWriter
{
  public:
    IoWriter(const std::string& report_name)
        : m_report_name(report_name) {}
    virtual ~IoWriter() = default;
    // Not implemented by default, override by hdf5
    virtual void configure_group(const std::string& /*group_name*/) {}
    virtual void configure_attribute(const std::string& /*group_name*/,
                                     const std::string& /*attribute_name*/,
                                     const std::string& /*attribute_value*/) {}
    virtual void configure_dataset(const std::string& /*dataset_name*/,
                                   int /*total_steps*/,
                                   int /*total_elements*/) {}
    virtual void write(const std::vector<double>& buffer,
                       int steps_to_write,
                       int total_elements) = 0;
    virtual void write(const std::string& name, const std::vector<int>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<uint32_t>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<uint64_t>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<float>& buffer) = 0;
    virtual void write(const std::string& name, const std::vector<double>& buffer) = 0;
    virtual void close() = 0;

  protected:
    std::string m_report_name;
};