#include <iostream>
#include <algorithm>
#include <numeric>

#include "reportinglib.hpp"
#include "sonata_format.hpp"
#include "implementation_interface.hpp"

SonataFormat::SonataFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<cells_t> cells)
: ReportFormat(report_name, max_buffer_size, num_steps, cells) {
    index_pointers.resize(cells->size());
    m_ioWriter = IoWriter::create_IoWriter(IoWriter::Kind::HDF5, report_name);
}

void SonataFormat::prepare_dataset() {

    std::cout << "Preparing SonataFormat Dataset for report: " << m_reportName << std::endl;

    // Prepare /report and /spikes headers
    for(auto& kv: *m_cells) {
        // /report
        const std::vector<uint32_t > compartment_ids = kv.second.get_compartment_ids();
        element_ids.insert(element_ids.end(), compartment_ids.begin(), compartment_ids.end());
        node_ids.push_back(kv.first);

        // /spikes
        const std::vector<double*> spikes = kv.second.get_spike_timestamps();
        for(auto& timestamp: spikes) {

            spike_node_ids.push_back(kv.first);
            spike_timestamps.push_back(*timestamp);
        }
    }
    int compartment_offset = Implementation::get_offset(m_totalCompartments);
    //int compartment_offset = get_compartment_offset();
    std::cout << "Total compartments are: " << m_totalCompartments << " and compartment_offset is: " << compartment_offset << std::endl;

    // Prepare index pointers
    if(!index_pointers.empty()) {
        index_pointers[0] = compartment_offset;
    }
    for (int i = 1; i < index_pointers.size(); i++) {
        int previousGid = node_ids[i-1];
        index_pointers[i] = index_pointers[i-1] + m_cells->at(previousGid).get_num_compartments();
    }

    // We only write the headers if there are compartments/spikes to write
    if(m_totalCompartments > 0 ) {
        write_report_header();
    }

    if(m_totalSpikes > 0) {
        write_spikes_header();
    }
}
void SonataFormat::write_report_header() {

    std::cout << "Writing report header!" << std::endl;
    m_ioWriter->configure_group("/report");
    m_ioWriter->configure_group("/report/mapping");
    m_ioWriter->configure_dataset("/report/data", m_numSteps, m_totalCompartments);

    m_ioWriter->write("/report/mapping/node_ids", node_ids);
    m_ioWriter->write("/report/mapping/index_pointers", index_pointers);
    m_ioWriter->write("/report/mapping/element_ids", element_ids);
}

void SonataFormat::write_spikes_header() {

    std::cout << "Writing spike header!" << std::endl;
    m_ioWriter->configure_group("/spikes");
    m_ioWriter->configure_attribute("/spikes", "sorting");
    Implementation::sort_spikes(spike_timestamps, spike_node_ids);
    m_ioWriter->write("/spikes/timestamps", spike_timestamps);
    m_ioWriter->write("/spikes/node_ids", spike_node_ids);
}

void SonataFormat::write_data() {

    if(m_remainingSteps > 0) {
        std::cout << "Writing data from sonataFormat! " << std::endl;
        std::cout << "++Remaining steps: " << m_remainingSteps << std::endl;
        std::cout << "++Steps to write: " << m_steps_to_write << std::endl;
        std::cout << "++total compartments: " << m_totalCompartments << std::endl;
        if (m_remainingSteps < m_steps_to_write) {
            // Write remaining steps
            m_ioWriter->write("Data", m_reportBuffer, m_remainingSteps, m_numSteps, m_totalCompartments);
        } else {
            m_ioWriter->write("Data", m_reportBuffer, m_steps_to_write, m_numSteps, m_totalCompartments);
        }
    }
}

void SonataFormat::close() {
    m_ioWriter->close();
}
