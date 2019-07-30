#pragma once

#include <memory>
#include <string>
#include <map>

#include "node.hpp"
#include "report_format.hpp"

class Report {
  private:
    std::string m_report_name;
    int m_num_nodes;
    double m_tstart;
    double m_tend;
    double m_dt;
    int m_num_steps;
    size_t m_max_buffer_size;

    std::unique_ptr<ReportFormat> m_report_format;

  protected:
    using nodes_t = std::map<uint64_t, Node>;
    std::shared_ptr<nodes_t> m_nodes;

  public:
    enum class Kind {COMPARTMENT, SOMA, SPIKE};

    Report(const std::string& report_name, double tstart, double tend, double dt);
    virtual ~Report() = default;

    int get_num_nodes() const { return m_num_nodes; }
    bool is_empty() const { return m_nodes->empty(); }
    /**
     * Allocates the buffers used to hold main
     * report data.
     *
     * @return 0 on success, non zero on failure
     */
    int prepare_dataset();

    static std::shared_ptr<Report> create_report(const std::string& report_name, double tstart,
                                                double tend, double dt, Report::Kind kind);
    void add_node(uint64_t node_id, uint64_t gid, uint64_t vgid);
    virtual int add_variable(uint64_t node_id, double* voltage) = 0;
    virtual size_t get_total_compartments() const = 0;

    virtual int record_data(double timestep, const std::vector<uint64_t>& node_ids);
    virtual int end_iteration(double timestep);
    virtual void flush(double time);
    int set_max_buffer_size(size_t buffer_size);
};