#include "report.hpp"

namespace bbp {
namespace sonata {

class SomaReport: public Report
{
  public:
    SomaReport(const std::string& reportName, double tstart, double tend, double dt);

    void add_node(uint64_t node_id) override;
    size_t get_total_elements() const noexcept override;
};

}  // namespace sonata
}  // namespace bbp
