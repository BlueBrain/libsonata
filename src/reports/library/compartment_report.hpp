#include "report.hpp"

class CompartmentReport: public Report {

  public:
    CompartmentReport(const std::string& reportName, double tstart, double tend, double dt);

    size_t get_total_compartments() override;
    int add_variable(int cell_number, double* pointer) override;
};