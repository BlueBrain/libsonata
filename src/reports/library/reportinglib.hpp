#pragma once

#include <map>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include "report.hpp"

/**
 *  \brief Contains and manages the reports
 */
class ReportingLib {
    using reports_t = std::map<std::string, std::shared_ptr<Report>>;
#ifdef HAVE_MPI
    using communicators_t = std::map<std::string, MPI_Comm>;
#endif
  private:
    reports_t m_reports;

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

    int get_num_reports() const;

    std::shared_ptr<Report> create_report(const std::string& name, const std::string& king,
                                          double tstart, double tend, double dt);

    std::shared_ptr<Report> get_report(const std::string& name) const;

    bool report_exists(const std::string& name) const;

    void make_global_communicator();
    void prepare_datasets();

    void write_spikes(const std::vector<double>& spike_timestamps, const std::vector<int>& spike_node_ids);

    template <typename T>
    void apply_all(void (Report::*fun)(T), T data) {
        std::for_each(m_reports.begin(), m_reports.end(),
                [&](reports_t::value_type arg){((*(arg.second)).*fun)(data);});
    }
};
