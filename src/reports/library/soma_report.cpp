#include <iostream>

#include "soma_report.hpp"

SomaReport::SomaReport(const std::string& report_name, double tstart, double tend, double dt)
: Report(report_name, tstart, tend, dt) {}

size_t SomaReport::get_total_compartments() {
    // Every cell has only 1 compartment on a soma report
    return m_cells->size();
}

int SomaReport::add_variable(int cellnumber, double* pointer) {
    cells_t::iterator cellFinder = m_cells->find(cellnumber);
    if (cellFinder != m_cells->end()) {
        if(cellFinder->second.get_num_compartments() == 0) {
            cellFinder->second.add_compartment(pointer);
        } else {
            std::cerr << "ERROR: Soma report cells can only have 1 compartment" << std::endl;
            return -1;
        }
    } else {
        std::cerr << "ERROR: Searching this cell: " << cellnumber << std::endl;
        return -1;
    }
    return 0;
}
