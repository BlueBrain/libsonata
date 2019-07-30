#include <iostream>
#include <cmath>
#include <limits>

#include "report.hpp"
#include "soma_report.hpp"
#include "compartment_report.hpp"
#include "spike_report.hpp"

#define DEFAULT_MAX_BUFFER_SIZE 1024

Report::Report(const std::string& report_name, double tstart, double tend, double dt)
: m_report_name(report_name), m_tstart(tstart), m_tend(tend), m_dt(dt), m_num_nodes(0) {
    m_nodes = std::make_shared<nodes_t>();
    // Calculate number of reporting steps
    m_num_steps = static_cast<int>((tend - tstart) / dt);
    if (std::fabs(m_num_steps * dt + tstart - tend) > std::numeric_limits<float>::epsilon()) {
        m_num_steps++;
    }
    // Default max buffer size
    m_max_buffer_size = DEFAULT_MAX_BUFFER_SIZE;
    std::cout << "Creating report " << m_report_name << std::endl;
    std::cout << "+Number of steps = " << m_num_steps << std::endl;
}

std::shared_ptr<Report> Report::create_report(const std::string& report_name, double tstart,
                                             double tend, double dt, Report::Kind kind) {
    switch(kind) {
        case Report::Kind::COMPARTMENT:
            return std::make_shared<CompartmentReport>(report_name, tstart, tend, dt);
        case Report::Kind::SOMA:
            return std::make_shared<SomaReport>(report_name, tstart, tend, dt);
        case Report::Kind::SPIKE:
            return std::make_shared<SpikeReport>(report_name, tstart, tend, dt);
        default:
            std::cout << "Report type '" << static_cast<int>(kind) << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}

void Report::add_node(uint64_t node_id, uint64_t gid, uint64_t vgid) {

    if (m_nodes->find(node_id) == m_nodes->end()) {
        // node is new insert it into the map
        m_nodes->insert(std::make_pair(node_id, Node(gid, vgid)));
        m_num_nodes++;
        std::cout << "Report: Added node " << gid << std::endl;
    } else {
        std::cerr << "Warning: attempted to add node " << node_id
                  << " to the target multiple time on same node.  Ignoring." << std::endl;
    }
}

int Report::prepare_dataset() {
    m_report_format = ReportFormat::create_report_format(m_report_name, m_max_buffer_size, m_num_steps, m_nodes, SONATA);
    m_report_format->prepare_dataset();
}

int Report::record_data(double timestep, const std::vector<uint64_t>& node_ids) {
    m_report_format->record_data(timestep, node_ids);
}

int Report::end_iteration(double timestep) {
    m_report_format->update_timestep(timestep);
}

void Report::flush(double time) {

    // Write if there are any remaining steps to write
    m_report_format->write_data();
    if(time - m_tend + m_dt / 2 > 1e-6) {
        m_report_format->close();
    }
}

int Report::set_max_buffer_size(size_t buffer_size) {
    std::cout << "Setting buffer size to " << buffer_size << std::endl;
    m_max_buffer_size = buffer_size;
}