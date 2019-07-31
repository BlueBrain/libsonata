#include <iostream>
#include <algorithm>

#include <reports/library/reportinglib.hpp>
#include <reports/library/implementation_interface.hpp>
#include <reports/io/hdf5_writer.hpp>
#include "sonata_data.hpp"

SonataData::SonataData(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes)
: m_report_name(report_name), m_num_steps(num_steps), m_nodes(nodes), m_last_position(0), m_current_step(0),
m_total_elements(0), m_total_spikes(0), m_remaining_steps(0), m_buffer_size(0), m_steps_to_write(0) {

    std::cout << "Creating SonataData for " << report_name << std::endl;
    prepare_buffer(max_buffer_size);
    m_index_pointers.resize(nodes->size());

    m_io_writer = std::make_unique<HDF5Writer>(report_name);
}

SonataData::~SonataData() {
    if(m_buffer_size > 0) {
        delete[] m_report_buffer;
    }
}

void SonataData::prepare_buffer(size_t max_buffer_size) {

    std::cout << "Prepare buffer for " << m_report_name << std::endl;

    for (auto& kv : *m_nodes) {
        m_total_elements += kv.second.get_num_elements();
        m_total_spikes += kv.second.get_num_spikes();
    }

    if(m_total_elements > 0) {
        // Calculate the timesteps that fit given a buffer size
        int max_steps_to_write = max_buffer_size / sizeof(float) / m_total_elements;
        if (max_steps_to_write < m_num_steps) {
            // Minimum 1 timestep required to write
            m_steps_to_write = max_steps_to_write > 0? max_steps_to_write: 1;
        } else {
            // If the buffer size is bigger that all the timesteps needed to record we allocate only the amount of timesteps
            m_steps_to_write = m_num_steps;
        }

        m_remaining_steps = m_num_steps;

        std::cout << "max_buffer_size: " << max_buffer_size << std::endl;
        std::cout << "m_totalelements: " << m_total_elements << std::endl;
        std::cout << "Steps to write: " << m_steps_to_write << std::endl;
        //m_bufferSize = m_totalelements * m_numSteps;
        m_buffer_size = m_total_elements * m_steps_to_write;
        m_report_buffer = new double[m_buffer_size];
        std::cout << "Buffer size: " << m_buffer_size << std::endl;
    }
}

int SonataData::record_data(double timestep, const std::vector<uint64_t>& node_ids) {
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

int SonataData::update_timestep(double timestep) {

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

void SonataData::prepare_dataset() {

    std::cout << "Preparing SonataData Dataset for report: " << m_report_name << std::endl;
    // Prepare /report and /spikes headers
    for(auto& kv: *m_nodes) {
        // /report
        const std::vector<uint32_t> element_ids = kv.second.get_element_ids();
        m_element_ids.insert(m_element_ids.end(), element_ids.begin(), element_ids.end());
        m_node_ids.push_back(kv.first);

        // /spikes
        const std::vector<double*> spikes = kv.second.get_spike_timestamps();
        for(auto& timestamp: spikes) {

            m_spike_node_ids.push_back(kv.first);
            m_spike_timestamps.push_back(*timestamp);
        }
    }
    int element_offset = Implementation::get_offset(m_total_elements);
    std::cout << "Total elements are: " << m_total_elements << " and element_offset is: " << element_offset << std::endl;

    // Prepare index pointers
    if(!m_index_pointers.empty()) {
        m_index_pointers[0] = element_offset;
    }
    for (int i = 1; i < m_index_pointers.size(); i++) {
        int previous_gid = m_node_ids[i-1];
        m_index_pointers[i] = m_index_pointers[i-1] + m_nodes->at(previous_gid).get_num_elements();
    }

    // We only write the headers if there are elements/spikes to write
    if(m_total_elements > 0 ) {
        write_report_header();
    }

    if(m_total_spikes > 0) {
        write_spikes_header();
    }
}
void SonataData::write_report_header() {
    //TODO: remove configure_group and add it to write_any()
    std::cout << "Writing report header!" << std::endl;
    m_io_writer->configure_group("/report");
    m_io_writer->configure_group("/report/mapping");
    m_io_writer->configure_dataset("/report/data", m_num_steps, m_total_elements);

    m_io_writer->write("/report/mapping/node_ids", m_node_ids);
    m_io_writer->write("/report/mapping/index_pointers", m_index_pointers);
    m_io_writer->write("/report/mapping/element_ids", m_element_ids);
}

void SonataData::write_spikes_header() {

    std::cout << "Writing spike header!" << std::endl;
    m_io_writer->configure_group("/spikes");
    m_io_writer->configure_attribute("/spikes", "sorting", "time");
    Implementation::sort_spikes(m_spike_timestamps, m_spike_node_ids);
    m_io_writer->write("/spikes/timestamps", m_spike_timestamps);
    m_io_writer->write("/spikes/node_ids", m_spike_node_ids);
}

void SonataData::write_data() {

    if(m_remaining_steps > 0) {
        std::cout << "Writing data from SonataData! " << std::endl;
        std::cout << "++Remaining steps: " << m_remaining_steps << std::endl;
        std::cout << "++Steps to write: " << m_steps_to_write << std::endl;
        std::cout << "++total elements: " << m_total_elements << std::endl;
        if (m_remaining_steps < m_steps_to_write) {
            // Write remaining steps
            m_io_writer->write(m_report_buffer, m_remaining_steps, m_num_steps, m_total_elements);
        } else {
            m_io_writer->write(m_report_buffer, m_steps_to_write, m_num_steps, m_total_elements);
        }
    }
}

void SonataData::close() {
    m_io_writer->close();
}

