#include <iostream>
#include <algorithm>
#include <numeric>

#include "reportinglib.hpp"
#include "sonata_format.hpp"
#include "implementation_interface.hpp"

SonataFormat::SonataFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes)
: ReportFormat(report_name, max_buffer_size, num_steps, nodes) {
    index_pointers.resize(nodes->size());
    //// TODO
    m_io_writer = IoWriter::create_io_writer(IoWriter::Kind::HDF5, report_name);
}

void SonataFormat::prepare_dataset() {

    std::cout << "Preparing SonataFormat Dataset for report: " << m_report_name << std::endl;

    // Prepare /report and /spikes headers
    for(auto& kv: *m_nodes) {
        // /report
        const std::vector<uint32_t> compartment_ids = kv.second.get_compartment_ids();
        element_ids.insert(element_ids.end(), compartment_ids.begin(), compartment_ids.end());
        node_ids.push_back(kv.first);

        // /spikes
        const std::vector<double*> spikes = kv.second.get_spike_timestamps();
        for(auto& timestamp: spikes) {

            spike_node_ids.push_back(kv.first);
            spike_timestamps.push_back(*timestamp);
        }
    }
    int compartment_offset = Implementation::get_offset(m_total_compartments);
    std::cout << "Total compartments are: " << m_total_compartments << " and compartment_offset is: " << compartment_offset << std::endl;

    // Prepare index pointers
    if(!index_pointers.empty()) {
        index_pointers[0] = compartment_offset;
    }
    for (int i = 1; i < index_pointers.size(); i++) {
        int previous_gid = node_ids[i-1];
        index_pointers[i] = index_pointers[i-1] + m_nodes->at(previous_gid).get_num_compartments();
    }

    // We only write the headers if there are compartments/spikes to write
    if(m_total_compartments > 0 ) {
        write_report_header();
    }

    if(m_total_spikes > 0) {
        write_spikes_header();
    }
}
void SonataFormat::write_report_header() {
    //TODO: remove configure_group and add it to write_any()
    std::cout << "Writing report header!" << std::endl;
    m_io_writer->configure_group("/report");
    m_io_writer->configure_group("/report/mapping");
    m_io_writer->configure_dataset("/report/data", m_num_steps, m_total_compartments);

    m_io_writer->write("/report/mapping/node_ids", node_ids);
    m_io_writer->write("/report/mapping/index_pointers", index_pointers);
    m_io_writer->write("/report/mapping/element_ids", element_ids);
}

void SonataFormat::write_spikes_header() {

    std::cout << "Writing spike header!" << std::endl;
    m_io_writer->configure_group("/spikes");
    m_io_writer->configure_attribute("/spikes", "sorting");
    Implementation::sort_spikes(spike_timestamps, spike_node_ids);
    m_io_writer->write("/spikes/timestamps", spike_timestamps);
    m_io_writer->write("/spikes/node_ids", spike_node_ids);
}

void SonataFormat::write_data() {

    if(m_remaining_steps > 0) {
        std::cout << "Writing data from sonataFormat! " << std::endl;
        std::cout << "++Remaining steps: " << m_remaining_steps << std::endl;
        std::cout << "++Steps to write: " << m_steps_to_write << std::endl;
        std::cout << "++total compartments: " << m_total_compartments << std::endl;
        if (m_remaining_steps < m_steps_to_write) {
            // Write remaining steps
            m_io_writer->write(m_report_buffer, m_remaining_steps, m_num_steps, m_total_compartments);
        } else {
            m_io_writer->write(m_report_buffer, m_steps_to_write, m_num_steps, m_total_compartments);
        }
    }
}

void SonataFormat::close() {
    m_io_writer->close();
}
