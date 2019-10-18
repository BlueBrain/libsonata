#include <iostream>
#include <algorithm>

#include <reports/library/reportinglib.hpp>
#include <reports/library/implementation_interface.hpp>
#include <reports/io/hdf5_writer.hpp>
#include "sonata_data.hpp"

SonataData::SonataData(const std::string& report_name, size_t max_buffer_size, int num_steps, double dt, double tstart, double tend, std::shared_ptr<nodes_t> nodes)
: m_report_name(report_name), m_num_steps(num_steps), m_nodes(nodes), m_last_position(0), m_current_step(0),
m_total_elements(0), m_total_spikes(0), m_remaining_steps(0), m_buffer_size(0), m_steps_to_write(0), m_steps_recorded(0) {

    prepare_buffer(max_buffer_size);
    m_index_pointers.resize(nodes->size());

    m_reporting_period = static_cast<int> (dt / ReportingLib::m_atomic_step);
    m_last_step_recorded = tstart / ReportingLib::m_atomic_step;
    m_last_step = tend / ReportingLib::m_atomic_step;

    m_io_writer = std::make_unique<HDF5Writer>(report_name);
}

SonataData::~SonataData() {
    if(m_buffer_size > 0) {
        delete[] m_report_buffer;
    }
}

void SonataData::prepare_buffer(size_t max_buffer_size) {
    logger->trace("Prepare buffer for {}", m_report_name);
    for (auto& kv : *m_nodes) {
        m_total_elements += kv.second.get_num_elements();
        m_total_spikes += kv.second.get_num_spikes();
    }

    if(m_total_elements > 0) {
        // Calculate the timesteps that fit given a buffer size
        int max_steps_to_write = max_buffer_size / sizeof(double) / m_total_elements;
        if (max_steps_to_write < m_num_steps) {
            if(max_steps_to_write < ReportingLib::m_min_steps_to_record) {
                m_steps_to_write = ReportingLib::m_min_steps_to_record;
            } else {
                // Minimum 1 timestep required to write
                m_steps_to_write = max_steps_to_write > 0? max_steps_to_write: 1;
            }
        } else {
            // If the buffer size is bigger that all the timesteps needed to record we allocate only the amount of timesteps
            m_steps_to_write = m_num_steps;
        }

        m_remaining_steps = m_num_steps;

        if(ReportingLib::m_rank == 0) {
            logger->info("-Total elements: {}", m_total_elements);
            logger->info("-Num steps: {}", m_num_steps);
            logger->info("-Steps to write: {}", m_steps_to_write);
            logger->info("-Max Steps to write: {}", max_steps_to_write);
            logger->info("-Max Buffer size: {}", max_buffer_size);
        }
        m_buffer_size = m_total_elements * (m_steps_to_write);
        m_report_buffer = new double[m_buffer_size];
        if(ReportingLib::m_rank == 0) {
            logger->info("-Buffer size: {}", m_buffer_size);
        }
    }
}

bool SonataData::is_due_to_report(double step) {
    // Dont record data if current step < tstart
    if(step < m_last_step_recorded) {
        return false;
    // Dont record data if current step > tend   
    } else if (step > m_last_step ) {
        return false;
    // Dont record data if is not a reporting step (step%period)
    } else if(static_cast<int>(step-m_last_step_recorded) % m_reporting_period != 0) {
        return false;
    }
    return true;
}

void SonataData::record_data(double step, const std::vector<uint64_t>& node_ids) {
    // Calculate the offset to write into the buffer
    int offset = static_cast<int> ((step-m_last_step_recorded)/m_reporting_period);
    int local_position = m_last_position + m_total_elements * offset;
    if(ReportingLib::m_rank == 0) {
        logger->trace("RANK={} Recording data for step={} last_step_recorded={} first GID={} buffer_size={} and offset={}",
                    ReportingLib::m_rank, step, m_last_step_recorded, node_ids[0], m_buffer_size, local_position);
    }
    int written;
    for (auto &kv: *m_nodes) {
        int current_gid = kv.first;
        if(node_ids.size() == m_nodes->size()) {
            // Record every node
            written = kv.second.fill_data(&m_report_buffer[local_position], true);
        } else {
            // Check if node is set to be recorded (found in nodeids)
            bool node_to_be_recorded = std::find(node_ids.begin(), node_ids.end(), current_gid) != node_ids.end();
            written = kv.second.fill_data(&m_report_buffer[local_position], node_to_be_recorded);
        }
        local_position += kv.second.get_num_elements();
    }
    m_nodes_recorded.insert(node_ids.begin(), node_ids.end());
    // Increase steps recorded when all nodes from specific rank has been already recorded
    if(m_nodes_recorded.size() == m_nodes->size()) {
        m_steps_recorded++;
    }
}

void SonataData::record_data(double step) {
    int local_position = m_last_position;
    if(ReportingLib::m_rank == 0) {
        logger->trace("RANK={} Recording data for step={} last_step_recorded={} buffer_size={} and offset={}", 
                    ReportingLib::m_rank, step, m_last_step_recorded, m_buffer_size, local_position);
    }
    
    int written;
    for (auto &kv: *m_nodes) {
        int current_gid = kv.first;
        written = kv.second.fill_data(&m_report_buffer[local_position], true);
        local_position += kv.second.get_num_elements();
    }
    
    m_current_step++;
    m_last_position += m_total_elements;    
    m_last_step_recorded += m_reporting_period;
    
    if(m_current_step == m_steps_to_write) {
        write_data();
    }
}

void SonataData::update_timestep(double timestep) {
    if(m_remaining_steps > 0) {
        if(ReportingLib::m_rank == 0) {
            logger->trace("Updating timestep t={}", timestep);
        }
        m_current_step += m_steps_recorded;
        m_last_position += m_total_elements * m_steps_recorded;
        m_last_step_recorded += m_reporting_period * m_steps_recorded;
        m_nodes_recorded.clear();
        
        // Write when buffer is full, finish all remaining recordings or when record several steps in a row
        if(m_current_step == m_steps_to_write || m_current_step == m_remaining_steps || m_steps_recorded > 1) {
            if(ReportingLib::m_rank == 0) {
                logger->trace("Writing to file {}! steps_to_write={}, current_step={}, remaining_steps={}", m_report_name, m_steps_to_write, m_current_step, m_remaining_steps);
            }    
            write_data();
        }
        m_steps_recorded = 0;
    }
}

void SonataData::prepare_dataset(bool spike_report) {
    logger->trace("Preparing SonataData Dataset for report: {}", m_report_name);
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
    int element_offset = Implementation::get_offset(m_report_name, m_total_elements);
    logger->trace("Total elements are: {} and element offset is: {}", m_total_elements, element_offset);

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

    if(spike_report) {
        write_spikes_header();
    }
}
void SonataData::write_report_header() {
    //TODO: remove configure_group and add it to write_any()
    logger->trace("Writing REPORT header!");
    m_io_writer->configure_group("/report");
    m_io_writer->configure_group("/report/mapping");
    m_io_writer->configure_dataset("/report/data", m_num_steps, m_total_elements);

    m_io_writer->write("/report/mapping/node_ids", m_node_ids);
    m_io_writer->write("/report/mapping/index_pointers", m_index_pointers);
    m_io_writer->write("/report/mapping/element_ids", m_element_ids);
}

void SonataData::write_spikes_header() {

    logger->trace("Writing SPIKE header!");
    m_io_writer->configure_group("/spikes");
    m_io_writer->configure_attribute("/spikes", "sorting", "time");
    Implementation::sort_spikes(m_spike_timestamps, m_spike_node_ids);
    m_io_writer->write("/spikes/timestamps", m_spike_timestamps);
    m_io_writer->write("/spikes/node_ids", m_spike_node_ids);
}

void SonataData::write_data() {
    if(m_remaining_steps > 0) {
        m_io_writer->write(m_report_buffer, m_current_step, m_num_steps, m_total_elements);
        m_remaining_steps -= m_current_step;
        if(ReportingLib::m_rank == 0) {
            logger->info("Writing timestep data to file {}", m_report_name);
            logger->info("-Steps written: {}", m_current_step);
            logger->info("-Remaining steps: {}", m_remaining_steps);
        }
        m_last_position = 0;
        m_current_step = 0;
    }
}

void SonataData::close() {
    m_io_writer->close();
}

