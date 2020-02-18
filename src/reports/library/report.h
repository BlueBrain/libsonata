#pragma once
#include <map>
#include <memory>
#include <string>

#include "../data/node.h"
#include "../data/sonata_data.h"

namespace bbp {
namespace sonata {

class Report
{
  public:
    Report(const std::string& report_name, double tstart, double tend, double dt);
    int get_num_nodes(const std::string& population_name) const {
        return populations_->at(population_name)->size();
    }
    bool is_empty() const noexcept {
        return populations_->empty();
    }
    int get_num_steps() const noexcept {
        return num_steps_;
    }
    /**
     * Allocates the buffers used to hold main
     * report data.
     *
     * @return 0 on success, non zero on failure
     */
    int prepare_dataset();

    virtual void add_node(const std::string& population_name, uint64_t node_id);
    bool node_exists(const std::string& population_name, uint64_t node_id) const;
    bool population_exists(const std::string& population_name) const;
    std::shared_ptr<Node> get_node(const std::string& population_name, uint64_t node_id) const;
    virtual size_t get_total_elements(const std::string& population_name) const = 0;

    virtual void record_data(double step, const std::vector<uint64_t>& node_ids);
    virtual void record_data(double step);
    virtual void check_and_flush(double timestep);
    virtual void flush(double time);
    void refresh_pointers(std::function<double*(double*)> refresh_function);
    void set_max_buffer_size(size_t buffer_size);

  protected:
    using nodes_t = std::map<uint64_t, std::shared_ptr<Node>>;
    using populations_t = std::map<std::string, std::shared_ptr<nodes_t>>;
    std::shared_ptr<populations_t> populations_;
    std::vector<std::shared_ptr<SonataData>> sonata_populations_;

  private:
    std::string report_name_;
    double tstart_;
    double tend_;
    double dt_;
    int num_steps_;
    size_t max_buffer_size_;
    bool report_is_closed_;
};

}  // namespace sonata
}  // namespace bbp
