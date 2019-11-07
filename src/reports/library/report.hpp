#pragma once
#include <memory>
#include <string>
#include <map>

#include <reports/data/sonata_data.hpp>
#include <reports/data/node.hpp>

class Report {
  public:
    Report(const std::string& report_name, double tstart, double tend, double dt);
    virtual ~Report() = default;
    int get_num_nodes() const noexcept { return m_nodes->size(); }
    bool is_empty() const noexcept { return m_nodes->empty(); }
    /**
     * Allocates the buffers used to hold main
     * report data.
     *
     * @return 0 on success, non zero on failure
     */
    int prepare_dataset();

    virtual void add_node(uint64_t node_id, uint64_t gid);
    bool node_exists(uint64_t node_id) const;
    std::shared_ptr<Node> get_node(uint64_t node_id);
    virtual size_t get_total_elements() const = 0;

    virtual void record_data(double step, const std::vector<uint64_t>& node_ids);
    virtual void record_data(double step);
    virtual void end_iteration(double timestep);
    virtual void flush(double time);
    void refresh_pointers(std::function<double*(double*)> refresh_function);
    void set_max_buffer_size(size_t buffer_size);

  protected:
    using nodes_t = std::map<uint64_t, std::shared_ptr<Node>>;
    std::shared_ptr<nodes_t> m_nodes;
    std::unique_ptr<SonataData> m_sonata_data;

  private:
    std::string m_report_name;
    double m_tstart;
    double m_tend;
    double m_dt;
    int m_num_steps;
    size_t m_max_buffer_size;
    bool m_report_is_closed;
};
