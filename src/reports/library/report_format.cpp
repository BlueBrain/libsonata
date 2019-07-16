#include <algorithm>
#include <stdlib.h>
#include <new>
#include <iostream>
#include <stdlib.h>

#include "report_format.hpp"
#include "sonata_format.hpp"

ReportFormat::ReportFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<cells_t> cells)
: m_reportName(report_name), m_numSteps(num_steps), m_cells(cells), m_lastPosition(0), m_currentStep(0),
m_totalCompartments(0), m_totalSpikes(0), m_remainingSteps(0), m_bufferSize(0), m_steps_to_write(0) {

    std::cout << "Creating ReportFormat for " << report_name << std::endl;
    prepare_buffer(max_buffer_size);
}

ReportFormat::~ReportFormat() {
    if(m_bufferSize > 0) {
        delete[] m_reportBuffer;
    }
}

void ReportFormat::prepare_buffer(size_t max_buffer_size) {

    std::cout << "Prepare buffer for " << m_reportName << std::endl;

    for (auto& kv : *m_cells) {
        m_totalCompartments += kv.second.get_num_compartments();
        m_totalSpikes += kv.second.get_num_spikes();
    }

    if(m_totalCompartments > 0) {
        // Calculate the timesteps that fit given a buffer size
        int max_steps_to_write = max_buffer_size / sizeof(float) / m_totalCompartments;
        if (max_steps_to_write < m_numSteps) {
            // Minimum 1 timestep required to write
            m_steps_to_write = max_steps_to_write > 0?max_steps_to_write: 1;
        } else {
            // If the buffer size is bigger that all the timesteps needed to record we allocate only the amount of timesteps
            m_steps_to_write = m_numSteps;
        }

        m_remainingSteps = m_numSteps;

        std::cout << "max_buffer_size: " << max_buffer_size << std::endl;
        std::cout << "m_totalCompartments: " << m_totalCompartments << std::endl;
        std::cout << "Steps to write: " << m_steps_to_write << std::endl;
        //m_bufferSize = m_totalCompartments * m_numSteps;
        m_bufferSize = m_totalCompartments * m_steps_to_write;
        m_reportBuffer = new float[m_bufferSize];
        std::cout << "Buffer size: " << m_bufferSize << std::endl;
    }
}

std::unique_ptr<ReportFormat> ReportFormat::create_ReportFormat(const std::string& report_name, size_t max_buffer_size,
        int num_steps, std::shared_ptr<cells_t> cells, const KindFormat& kind) {

    switch(kind) {
        case SONATA:
            return std::make_unique<SonataFormat>(report_name, max_buffer_size, num_steps, cells);
        case BINARY:
            std::cout << "Binary report format not implemented yet!" << std::endl;
            break;
        default:
            std::cout << "Report format type '" << kind << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}

int ReportFormat::record_data(double timestep, int ncells, int* cellids) {
    std::cout << "Recording data..." << std::endl;
    int local_position = m_lastPosition;
    int written;
    std::vector<int> node_ids(cellids, cellids + ncells);
    for(auto& kv: *m_cells) {
        int currentGid = kv.first;
        // Check if cell is set to be recorded (found in cellids)
        bool cell_to_be_recorded = std::find(node_ids.begin(), node_ids.end(), currentGid) != node_ids.end();
        written = kv.second.fill_data(&m_reportBuffer[local_position], cell_to_be_recorded);
        local_position += written;
    }
    m_lastPosition = local_position;

    // print_buffer();
}

int ReportFormat::update_timestep(double timestep) {

    m_currentStep++;
    if(m_currentStep == m_steps_to_write) {
        write_data();
        m_lastPosition = 0;
        m_currentStep = 0;
        m_remainingSteps-=m_steps_to_write;
        std::cout << "--Remaining steps: " << m_remainingSteps << std::endl;
        std::cout << "--Steps to write: " << m_steps_to_write << std::endl;
    }
}

void ReportFormat::print_buffer() {
    std::cout << "Report buffer size: " << m_bufferSize << std::endl;
    std::cout << "Report buffer:" << std::endl;
    for(int i=0; i<m_bufferSize; i++) {
        std::cout << m_reportBuffer[i] << ", ";
    }
    std::cout << std::endl;
}
