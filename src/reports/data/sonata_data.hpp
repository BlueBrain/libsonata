#pragma once
#include <map>
#include <set>

#include <reports/io/io_writer.hpp>
#include "node.hpp"

namespace bbp {
namespace sonata {

class SonataData {
    using nodes_t = std::map<uint64_t, std::shared_ptr<Node>>;
  public:
    SonataData(const std::string& report_name, size_t max_buffer_size, int num_steps, double dt, double tstart, double tend, std::shared_ptr<nodes_t> nodes);
    SonataData(const std::string& report_name, const std::vector<double>& spike_timestamps, const std::vector<int>& spike_node_ids);

    void prepare_dataset();
    void write_report_header();
    void write_spikes_header();
    void write_data();
    void close();

    bool is_due_to_report(double step) const;
    void record_data(double step, const std::vector<uint64_t>& node_ids);
    void record_data(double step);
    void update_timestep(double timestep);

    const std::vector<double>& get_report_buffer() const noexcept { return m_report_buffer; }

    const std::vector<uint64_t>& get_node_ids() const noexcept { return m_node_ids; }
    const std::vector<uint64_t>& get_index_pointers() const noexcept { return m_index_pointers; }
    const std::vector<uint32_t>& get_element_ids() const noexcept { return m_element_ids; }
    const std::vector<double>& get_spike_timestamps() const noexcept { return m_spike_timestamps; }
    const std::vector<int>& get_spike_node_ids() const noexcept { return m_spike_node_ids; }

  private:
    std::string m_report_name;
    std::vector<double> m_report_buffer;
    int m_total_elements = 0;
    int m_num_steps = 0;
    int m_steps_to_write = 0;
    int m_current_step = 0;
    int m_steps_recorded = 0;
    int m_last_position = 0;
    int m_remaining_steps = 0;
    int m_reporting_period = 0;
    double m_last_step_recorded = 0.;
    double m_last_step = 0.;

    std::vector<uint64_t> m_node_ids;
    std::vector<uint64_t> m_index_pointers;
    std::vector<uint32_t> m_element_ids;

    std::vector<double> m_spike_timestamps;
    std::vector<int> m_spike_node_ids;

    std::set<uint64_t> m_nodes_recorded;
    std::unique_ptr<IoWriter> m_io_writer;
    std::shared_ptr<nodes_t> m_nodes;

    void prepare_buffer(size_t max_buffer_size);
};

}
} // namespace bbp::sonata
