#include <cmath>
#include <iostream>
#include <limits>

#include "../utils/logger.h"
#include "report.h"
#include "sonatareport.h"

namespace bbp {
namespace sonata {

// 4MB
#define DEFAULT_MAX_BUFFER_SIZE 4194304

Report::Report(const std::string& report_name, double tstart, double tend, double dt)
    : populations_(std::make_shared<populations_t>())
    , report_name_(report_name)
    , tstart_(tstart)
    , tend_(tend)
    , dt_(dt)
    , max_buffer_size_(DEFAULT_MAX_BUFFER_SIZE)
    , report_is_closed_(false) {
    // Calculate number of reporting steps
    double sim_steps = (tend - tstart) / dt;
    num_steps_ = static_cast<int>(std::ceil(sim_steps));
}

void Report::add_node(const std::string& population_name, uint64_t node_id) {
    if (population_exists(population_name)) {
        if (node_exists(population_name, node_id)) {
            throw std::runtime_error("Warning: attempted to add node " + std::to_string(node_id) +
                                     " to the target multiple times on same node. Ignoring.");
        }
        std::shared_ptr<nodes_t> nodes = populations_->at(population_name);
        nodes->emplace(node_id, std::make_shared<Node>(node_id));
    } else {
        // node is new insert it into the map
        std::shared_ptr<nodes_t> nodes = std::make_shared<nodes_t>();
        nodes->emplace(node_id, std::make_shared<Node>(node_id));
        populations_->emplace(population_name, nodes);
    }
}

bool Report::node_exists(const std::string& population_name, uint64_t node_id) const {
    std::shared_ptr<nodes_t> nodes = populations_->at(population_name);
    return nodes->find(node_id) != nodes->end();
}

bool Report::population_exists(const std::string& population_name) const {
    return populations_->find(population_name) != populations_->end();
}

std::shared_ptr<Node> Report::get_node(const std::string& population_name, uint64_t node_id) const {
    return populations_->at(population_name)->at(node_id);
}

int Report::prepare_dataset() {
    for (const auto& population : *populations_) {
        std::shared_ptr<nodes_t> nodes = population.second;
        sonata_populations_.push_back(std::make_unique<SonataData>(report_name_,
                                                                   population.first,
                                                                   max_buffer_size_,
                                                                   num_steps_,
                                                                   dt_,
                                                                   tstart_,
                                                                   tend_,
                                                                   nodes));
        sonata_populations_.back()->prepare_dataset();
    }
    return 0;
}

void Report::record_data(double step, const std::vector<uint64_t>& node_ids) {
    for (const auto& sonata_data : sonata_populations_) {
        if (sonata_data->is_due_to_report(step)) {
            sonata_data->record_data(step, node_ids);
        }
    }
}

void Report::record_data(double step) {
    for (const auto& sonata_data : sonata_populations_) {
        if (sonata_data->is_due_to_report(step)) {
            sonata_data->record_data(step);
        }
    }
}

void Report::check_and_flush(double timestep) {
    for (const auto& sonata_data : sonata_populations_) {
        sonata_data->check_and_write(timestep);
    }
}

void Report::refresh_pointers(std::function<double*(double*)> refresh_function) {
    for (auto& population : *populations_) {
        std::shared_ptr<nodes_t> nodes = population.second;
        for (auto& kv : *nodes) {
            kv.second->refresh_pointers(refresh_function);
        }
    }
}

void Report::flush(double time) {
    if (SonataReport::rank_ == 0) {
        logger->trace("Flush() called at t={} for report {}", time, report_name_);
    }
    for (const auto& sonata_data : sonata_populations_) {
        // Write if there are any remaining steps to write
        sonata_data->write_data();
        if (time - tend_ + dt_ / 2 > 1e-6) {
            if (!report_is_closed_) {
                sonata_data->close();
                report_is_closed_ = true;
            }
        }
    }
}

void Report::set_max_buffer_size(size_t buffer_size) {
    logger->trace("Setting buffer size to {}", buffer_size);
    max_buffer_size_ = buffer_size;
}

}  // namespace sonata
}  // namespace bbp
