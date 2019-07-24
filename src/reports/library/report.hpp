#pragma once

#include <memory>
#include <string>
#include <map>

#include "cell.hpp"
#include "report_format.hpp"

class Report {
  private:
    std::string m_reportName;
    int m_numCells;
    double m_tstart;
    double m_tend;
    double m_dt;
    int m_numSteps;
    size_t m_max_buffer_size;

    std::unique_ptr<ReportFormat> m_reportFormat;

  protected:
    using cells_t = std::map<int, Cell>;
    std::shared_ptr<cells_t> m_cells;

  public:
    enum class Kind {COMPARTMENT, SOMA, SPIKE};

    Report(const std::string& report_name, double tstart, double tend, double dt);
    virtual ~Report() = default;

    int get_num_cells() { return m_numCells; }
    bool is_empty() { return m_cells->empty(); }
    /**
     * Allocates the buffers used to hold main
     * report data.
     *
     * @return 0 on success, non zero on failure
     */
    int prepare_dataset();

    static std::shared_ptr<Report> createReport(const std::string& report_name, double tstart,
                                                double tend, double dt, Report::Kind kind);
    void add_cell(int cell_number, unsigned long gid, unsigned long vgid);
    virtual int add_variable(int cell_number, double* pointer) = 0;
    virtual size_t get_total_compartments() = 0;

    virtual int recData(double timestep, int ncells, int* cellids);
    virtual int end_iteration(double timestep);
    virtual int flush(double time);
    int set_max_buffer_size(size_t buf_size);
};