#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include "report.hpp"

namespace bbp {
namespace sonata {

/**
 *  \brief Contains and manages the reports
 */
class SonataReport
{
    using reports_t = std::unordered_map<std::string, std::shared_ptr<Report>>;
#ifdef HAVE_MPI
    using communicators_t = std::unordered_map<std::string, MPI_Comm>;
#endif
  public:
    static double m_atomic_step;
    static double m_min_steps_to_record;
#ifdef HAVE_MPI
    static MPI_Comm m_has_nodes;
    static communicators_t m_communicators;
#endif
    static int m_rank;
    static bool first_report;

    /**
     * \brief Destroy all report objects.
     * This should invoke their destructor which will close the report
     * file and clean up
     */
    void clear();
    bool is_empty();

    int get_num_reports() const noexcept {
        return m_reports.size();
    }

    std::shared_ptr<Report> create_report(
        const std::string& name, const std::string& king, double tstart, double tend, double dt);

    std::shared_ptr<Report> get_report(const std::string& name) const;

    bool report_exists(const std::string& name) const;

    void create_communicators();
    void prepare_datasets();

    void write_spikes(const std::string& output_dir,
                      const std::vector<double>& spike_timestamps,
                      const std::vector<int>& spike_node_ids);

    template <typename Functor, typename T>
    void apply_all(const Functor& functor, T data) {
        std::for_each(m_reports.begin(), m_reports.end(), [&](reports_t::value_type arg) {
            functor(arg.second, data);
        });
    }

  private:
    reports_t m_reports;
};

}  // namespace sonata
}  // namespace bbp
