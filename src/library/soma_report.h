#pragma once
#include "report.h"

namespace bbp {
namespace sonata {

class SomaReport: public Report
{
  public:
    using Report::Report;

    void add_node(const std::string& population_name, uint64_t node_id) override;
    size_t get_total_elements(const std::string& population_name) const override;
};

}  // namespace sonata
}  // namespace bbp
