#pragma once
#include <map>

#include <reports/io/io_writer.hpp>
#include "node.hpp"

class SonataData {
  private:
    std::string m_report_name;
    double* m_report_buffer;
    size_t m_buffer_size;
    int m_total_elements;
    int m_total_spikes;
    int m_num_steps;
    int m_steps_to_write;
    int m_current_step;
    int m_last_position;
    int m_remaining_steps;
    int m_nodes_recorded;

    std::vector<uint64_t> m_node_ids;
    std::vector<uint64_t> m_index_pointers;
    std::vector<uint32_t> m_element_ids;

    std::vector<double> m_spike_timestamps;
    std::vector<int> m_spike_node_ids;

    std::unique_ptr<IoWriter> m_io_writer;
    using nodes_t = std::map<uint64_t, Node>;
    std::shared_ptr<nodes_t> m_nodes;

    void prepare_buffer(size_t max_buffer_size);

  public:
    SonataData(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes);
    ~SonataData();

    void prepare_dataset(bool spike_report);
    void write_report_header();
    void write_spikes_header();
    void write_data();
    void close();

    int record_data(double step, const std::vector<uint64_t>& node_ids, int period);
    int update_timestep(double timestep);

    int get_num_steps() const { return m_num_steps; }
    size_t get_buffer_size() const { return m_buffer_size; }
    double* get_report_buffer() const { return m_report_buffer; }

    const std::vector<uint64_t>& get_node_ids() const { return m_node_ids; }
    const std::vector<uint64_t>& get_index_pointers() const { return m_index_pointers; }
    const std::vector<uint32_t>& get_element_ids() const { return m_element_ids; }
};