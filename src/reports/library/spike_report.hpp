#include "report.hpp"

class SpikeReport: public Report {

  public:
    SpikeReport(const std::string& report_name, double tstart, double tend, double dt);

    size_t get_total_compartments() const override;
    int add_variable(uint64_t node_id, double* spike_value) override;

    // We dont need this on spike reports
    int record_data(double timestep, const std::vector<uint64_t>& node_ids) {}
    int end_iteration(double timestep) {}
    void flush(double time) {}
};