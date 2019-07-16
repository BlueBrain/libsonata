#include <stdlib.h>
#include <new>
#include <iostream>
#include <stdlib.h>

#include "spike_report.hpp"

SpikeReport::SpikeReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

int SpikeReport::get_total_compartments() {
    // Spike report doesnt have compartments
    return 0;
}

int SpikeReport::add_variable(int cellnumber, double* pointer) {
    cells_t::iterator cellFinder = m_cells->find(cellnumber);

    if (cellFinder != m_cells->end()) {
        cellFinder->second.add_spike(pointer);
    } else {
        std::cerr << "ERROR: Searching this cell: " << cellnumber << std::endl;
        return -1;
    }
    return 0;
}