#pragma once
#include "report.h"

namespace bbp {
namespace sonata {

class ElementReport: public Report
{
  public:
    ElementReport(const std::string& reportName, double tstart, double tend, double dt);
    size_t get_total_elements(const std::string& population_name) const override;
};

}  // namespace sonata
}  // namespace bbp
