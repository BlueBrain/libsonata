#include <stdlib.h>
#include <new>
#include <iostream>
#include <cmath>
#include <stdlib.h>

#include "report.hpp"
#include "soma_report.hpp"
#include "compartment_report.hpp"
#include "spike_report.hpp"

Report::Report(const std::string& report_name, double tstart, double tend, double dt)
: m_reportName(report_name), m_tstart(tstart), m_tend(tend), m_dt(dt), m_numCells(0) {
    m_cells = std::make_shared<cells_t>();
    // Calculate number of reporting steps
    m_numSteps = (int)std::floor((tend - tstart) / dt);
    if (std::fabs(m_numSteps * dt + tstart - tend) > 1e-9) {
        m_numSteps++;
    }
    // Default max buffer size
    m_max_buffer_size = 1024;
    std::cout << "Creating report " << m_reportName << std::endl;
    std::cout << "+Number of steps = " << m_numSteps << std::endl;
}

Report::~Report() {}

std::shared_ptr<Report> Report::createReport(const std::string& report_name, double tstart,
                                             double tend, double dt, KindReport kind) {
    switch(kind) {
        case COMPARTMENT:
            return std::make_shared<CompartmentReport>(report_name, tstart, tend, dt);
        case SOMA:
            return std::make_shared<SomaReport>(report_name, tstart, tend, dt);
        case SPIKE:
            return std::make_shared<SpikeReport>(report_name, tstart, tend, dt);
        default:
            std::cout << "Report type '" << kind << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}

void Report::add_cell(int cell_number, unsigned long gid, unsigned long vgid) {
    cells_t::iterator cellFinder = m_cells->find(cell_number);

    if (cellFinder == m_cells->end()) {
        // cell is new insert it into the map
        m_cells->insert(std::make_pair(cell_number, Cell(gid, vgid)));
        m_numCells++;
        std::cout << "Report: Added cell " << gid << std::endl;
    } else {
        std::cerr << "Warning: attempted to add cell " << cell_number
                  << " to the target multiple time on same node.  Ignoring." << std::endl;
    }
}

int Report::prepare_dataset() {
    m_reportFormat = ReportFormat::create_ReportFormat(m_reportName, m_max_buffer_size, m_numSteps, m_cells, SONATA);
    m_reportFormat->prepare_dataset();
}

int Report::recData(double timestep, int ncells, int* cellids) {
    m_reportFormat->record_data(timestep, ncells, cellids);
}

int Report::end_iteration(double timestep) {
    m_reportFormat->update_timestep(timestep);
}

int Report::flush(double time) {

    // Write if there are any remaining steps to write
    m_reportFormat->write_data();
    if(time - m_tend + m_dt / 2 > 1e-6) {
        m_reportFormat->close();
    }
}

int Report::set_max_buffer_size(size_t buf_size) {
    std::cout << "Setting buffer size to " << buf_size << std::endl;
    m_max_buffer_size = buf_size;
}