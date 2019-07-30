#include <iostream>
#include <algorithm>

#include "report_format.hpp"
#include "sonata_format.hpp"

ReportFormat::ReportFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes)
: m_report_name(report_name), m_num_steps(num_steps), m_nodes(nodes), m_last_position(0), m_current_step(0),
m_total_compartments(0), m_total_spikes(0), m_remaining_steps(0), m_buffer_size(0), m_steps_to_write(0) {

    std::cout << "Creating ReportFormat for " << report_name << std::endl;
    prepare_buffer(max_buffer_size);
}

ReportFormat::~ReportFormat() {
    if(m_buffer_size > 0) {
        delete[] m_report_buffer;
    }
}

void ReportFormat::prepare_buffer(size_t max_buffer_size) {

    std::cout << "Prepare buffer for " << m_report_name << std::endl;

    for (auto& kv : *m_nodes) {
        m_total_compartments += kv.second.get_num_compartments();
        m_total_spikes += kv.second.get_num_spikes();
    }

    if(m_total_compartments > 0) {
        // Calculate the timesteps that fit given a buffer size
        int max_steps_to_write = max_buffer_size / sizeof(float) / m_total_compartments;
        if (max_steps_to_write < m_num_steps) {
            // Minimum 1 timestep required to write
            m_steps_to_write = max_steps_to_write > 0? max_steps_to_write: 1;
        } else {
            // If the buffer size is bigger that all the timesteps needed to record we allocate only the amount of timesteps
            m_steps_to_write = m_num_steps;
        }

        m_remaining_steps = m_num_steps;

        std::cout << "max_buffer_size: " << max_buffer_size << std::endl;
        std::cout << "m_totalCompartments: " << m_total_compartments << std::endl;
        std::cout << "Steps to write: " << m_steps_to_write << std::endl;
        //m_bufferSize = m_totalCompartments * m_numSteps;
        m_buffer_size = m_total_compartments * m_steps_to_write;
        m_report_buffer = new double[m_buffer_size];
        std::cout << "Buffer size: " << m_buffer_size << std::endl;
    }
}

std::unique_ptr<ReportFormat> ReportFormat::create_report_format(const std::string& report_name, size_t max_buffer_size,
        int num_steps, std::shared_ptr<nodes_t> nodes, const KindFormat& kind) {

    switch(kind) {
        case SONATA:
            return std::make_unique<SonataFormat>(report_name, max_buffer_size, num_steps, nodes);
        case BINARY:
            std::cout << "Binary report format not implemented yet!" << std::endl;
            break;
        default:
            std::cout << "Report format type '" << kind << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}

int ReportFormat::record_data(double timestep, const std::vector<uint64_t>& node_ids) {
    std::cout << "Recording data..." << std::endl;
    int local_position = m_last_position;
    int written;
    for(auto& kv: *m_nodes) {
        int current_gid = kv.first;
        // Check if node is set to be recorded (found in nodeids)
        bool node_to_be_recorded = std::find(node_ids.begin(), node_ids.end(), current_gid) != node_ids.end();
        written = kv.second.fill_data(&m_report_buffer[local_position], node_to_be_recorded);
        local_position += written;
    }
    m_last_position = local_position;
    // print_buffer();
}

int ReportFormat::update_timestep(double timestep) {

    m_current_step++;
    if(m_current_step == m_steps_to_write) {
        write_data();
        m_last_position = 0;
        m_current_step = 0;
        m_remaining_steps -= m_steps_to_write;
        std::cout << "--Remaining steps: " << m_remaining_steps << std::endl;
        std::cout << "--Steps to write: " << m_steps_to_write << std::endl;
    }
}

void ReportFormat::print_buffer() {
    std::cout << "Report buffer size: " << m_buffer_size << std::endl;
    std::cout << "Report buffer:" << std::endl;
    for(int i=0; i<m_buffer_size; i++) {
        std::cout << m_report_buffer[i] << ", ";
    }
    std::cout << std::endl;
}
