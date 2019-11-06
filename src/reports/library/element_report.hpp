#include "report.hpp"

class ElementReport: public Report {
  public:
    ElementReport(const std::string& reportName, double tstart, double tend, double dt);
    size_t get_total_elements() const override;
};
