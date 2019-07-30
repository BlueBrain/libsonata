#pragma once
#include <map>

#include "node.hpp"
#include "io_writer.hpp"

enum KindFormat {SONATA, BINARY};

class ReportFormat {
  protected:
    std::string m_report_name;
    double* m_report_buffer;
    size_t m_buffer_size;
    int m_total_compartments;
    int m_total_spikes;
    int m_num_steps;
    int m_steps_to_write;
    int m_current_step;
    int m_last_position;
    int m_remaining_steps;

    std::unique_ptr<IoWriter> m_io_writer;

    using nodes_t = std::map<uint64_t, Node>;
    std::shared_ptr<nodes_t> m_nodes;

    void prepare_buffer(size_t max_buffer_size);

  public:
    ReportFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes);
    virtual ~ReportFormat();

    static std::unique_ptr<ReportFormat> create_report_format(const std::string& report_name, size_t max_buffer_size,
            int num_steps, std::shared_ptr<nodes_t> nodes, const KindFormat& kind);

    virtual void prepare_dataset() = 0;
    virtual void write_data() = 0;
    virtual void close() = 0;

    int record_data(double timestep, const std::vector<uint64_t>& node_ids);
    int update_timestep(double timestep);

    int get_num_steps() const { return m_num_steps; }
    size_t get_buffer_size() const { return m_buffer_size; }
    double* get_report_buffer() const { return m_report_buffer; }
    void print_buffer();
};