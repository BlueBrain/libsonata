#include "report.hpp"

class SpikeReport: public Report {

  public:
    SpikeReport(const std::string& report_name, double tstart, double tend, double dt);

    size_t get_total_elements() const override;
    int add_variable(uint64_t node_id, double* spike_value, uint32_t element_id) override;

    int prepare_sonata_dataset();
    // We dont need this on spike reports
    void record_data(double step, const std::vector<uint64_t>& node_ids) {}
    void record_data(double step) {}
    void end_iteration(double timestep) {}
    void flush(double time) {}
};
