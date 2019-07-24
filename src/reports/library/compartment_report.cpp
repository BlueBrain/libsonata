#include <iostream>

#include "compartment_report.hpp"

CompartmentReport::CompartmentReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t CompartmentReport::get_total_compartments() {
    size_t total = 0;
    for(auto& kv: *m_cells) {
        total += kv.second.get_num_compartments();
    }
    return total;
}

int CompartmentReport::add_variable(int cellnumber, double* pointer) {
    cells_t::iterator cellFinder = m_cells->find(cellnumber);
    if (cellFinder != m_cells->end()) {
        cellFinder->second.add_compartment(pointer);
    } else {
        std::cerr << "ERROR: Searching this cell: " << cellnumber << std::endl;
        return -1;
    }
    return 0;
}
