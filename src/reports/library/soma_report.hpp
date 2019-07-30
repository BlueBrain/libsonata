#include "report.hpp"

class SomaReport: public Report {

  public:
    SomaReport(const std::string& reportName, double tstart, double tend, double dt);

    size_t get_total_compartments() const override;
    int add_variable(uint64_t node_id, double* element_value) override;
};