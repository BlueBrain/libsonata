#include <iostream>
#include <cmath>
#include <limits>

#include <reports/utils/logger.hpp>
#include "report.hpp"
#include "reportinglib.hpp"

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
}

void Report::add_node(uint64_t node_id, uint64_t gid, uint64_t vgid) {

    if (m_nodes->find(node_id) == m_nodes->end()) {
        // node is new insert it into the map
        m_nodes->insert(std::make_pair(node_id, Node(gid, vgid)));
        m_num_nodes++;
        logger->trace("Added Node with GID {}", gid);
    } else {
        throw std::runtime_error("Warning: attempted to add node "+ std::to_string(node_id)+" to the target multiple time on same node. Ignoring.");
    }
}

int Report::prepare_dataset() {
    m_sonata_data = std::make_unique<SonataData>(m_report_name, m_max_buffer_size, m_num_steps, m_dt, m_tstart, m_nodes);
    return prepare_sonata_dataset();
}

int Report::prepare_sonata_dataset() {
    m_sonata_data->prepare_dataset(false);
    return 0;
}

void Report::record_data(double step, std::vector<uint64_t>& node_ids) {
    if(m_sonata_data->is_due_to_report(step)) {
        m_sonata_data->record_data(step, node_ids);
    }
}

void Report::end_iteration(double timestep) {
    m_sonata_data->update_timestep(timestep, false);
}

void Report::flush(double time) {

    // Write if there are any remaining steps to write
    m_sonata_data->write_data();
    if(time - m_tend + m_dt / 2 > 1e-6) {
        m_sonata_data->close();
    }
}

void Report::set_max_buffer_size(size_t buffer_size) {
    logger->trace("Setting buffer size to {}", buffer_size);
    m_max_buffer_size = buffer_size;
}

bool Report::check_add_variable(uint64_t node_id) {
    if (m_nodes->find(node_id) != m_nodes->end()) {
        return true;
    } else {
        throw std::runtime_error("ERROR: Searching this node: " + std::to_string(node_id));
    }
}
