#pragma once
#include <map>
#include <set>

#include "../io/hdf5_writer.h"
#include "node.h"

namespace bbp {
namespace sonata {

class SonataData
{
    using nodes_t = std::map<uint64_t, std::shared_ptr<Node>>;

  public:
    SonataData(const std::string& report_name,
               const std::string& population_name,
               size_t max_buffer_size,
               int num_steps,
               double dt,
               double tstart,
               double tend,
               std::shared_ptr<nodes_t> nodes);
    SonataData(const std::string& report_name,
               const std::string& population_name,
               const std::vector<double>& spike_timestamps,
               const std::vector<uint64_t>& spike_node_ids);

    void prepare_dataset();
    void write_report_header();
    void write_spikes_header(const std::string& order_by);
    void write_data();
    void close();

    bool is_due_to_report(double step) const noexcept;
    void record_data(double step, const std::vector<uint64_t>& node_ids);
    void record_data(double step);
    void check_and_write(double timestep);

    const std::vector<double>& get_report_buffer() const noexcept {
        return report_buffer_;
    }

    const std::vector<uint64_t>& get_node_ids() const noexcept {
        return node_ids_;
    }
    const std::vector<uint64_t>& get_index_pointers() const noexcept {
        return index_pointers_;
    }
    const std::vector<uint32_t>& get_element_ids() const noexcept {
        return element_ids_;
    }
    const std::vector<double>& get_spike_timestamps() const noexcept {
        return spike_timestamps_;
    }
    const std::vector<uint64_t>& get_spike_node_ids() const noexcept {
        return spike_node_ids_;
    }

  private:
    std::string report_name_;
    std::string population_name_;
    std::vector<double> report_buffer_;
    int total_elements_ = 0;
    int num_steps_ = 0;
    int steps_to_write_ = 0;
    int current_step_ = 0;
    int steps_recorded_ = 0;
    int last_position_ = 0;
    int remaining_steps_ = 0;
    int reporting_period_ = 0;
    double last_step_recorded_ = 0.;
    double last_step_ = 0.;

    std::vector<uint64_t> node_ids_;
    std::vector<uint64_t> index_pointers_;
    std::vector<uint32_t> element_ids_;

    std::vector<double> spike_timestamps_;
    std::vector<uint64_t> spike_node_ids_;

    std::set<uint64_t> nodes_recorded_;
    const std::unique_ptr<HDF5Writer> hdf5_writer_;
    std::shared_ptr<nodes_t> nodes_;

    void prepare_buffer(size_t max_buffer_size);
};

}  // namespace sonata
}  // namespace bbp
