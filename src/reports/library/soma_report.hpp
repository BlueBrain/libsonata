#include "report.hpp"

class SomaReport: public Report {

  public:
    SomaReport(const std::string& reportName, double tstart, double tend, double dt);

    size_t get_total_compartments() override;
    int add_variable(int cell_number, double* pointer) override;
};