#include "../utils/logger.h"
#include "sonatareport.h"
#include <bbp/sonata/reports.h>
#include <iostream>

bbp::sonata::SonataReport sonata_report;

int sonata_clear() {
    sonata_report.clear();
    return 0;
}

int sonata_create_report(
    const char* report_name, double tstart, double tend, double dt, const char* kind) {
    try {
        if (!sonata_report.report_exists(report_name)) {
            sonata_report.create_report(report_name, kind, tstart, tend, dt);
        }
    } catch (const std::exception& err) {
        logger->error(err.what());
        return -1;
    }
    return 0;
}

int sonata_add_node(const char* report_name, const char* population_name, uint64_t node_id) {
    if (!sonata_report.report_exists(report_name)) {
        return -2;
    }
    try {
        auto report = sonata_report.get_report(report_name);
        report->add_node(population_name, node_id);
    } catch (const std::exception& err) {
        logger->error(err.what());
        return -1;
    }
    return 0;
}

int sonata_add_element(const char* report_name,
                       const char* population_name,
                       uint64_t node_id,
                       uint32_t element_id,
                       double* voltage) {
    if (!sonata_report.report_exists(report_name)) {
        return -2;
    }
    try {
        auto report = sonata_report.get_report(report_name);
        auto node = report->get_node(population_name, node_id);
        node->add_element(voltage, element_id);
    } catch (const std::out_of_range& err) {
        logger->error(err.what());
        return -3;
    } catch (const std::exception& err) {
        logger->error(err.what());
        return -1;
    }
    return 0;
}

void sonata_setup_communicators() {
    sonata_report.create_communicators();
}

int sonata_prepare_datasets() {
    sonata_report.prepare_datasets();
    return 0;
}

void sonata_set_min_steps_to_record(int steps) {
    bbp::sonata::SonataReport::min_steps_to_record_ = steps;
}

int sonata_record_node_data(double step,
                            int num_nodes,
                            const int* nodeids,
                            const char* report_name) {
    if (sonata_report.is_empty()) {
        return -3;
    }
    if (!sonata_report.report_exists(report_name)) {
        return -1;
    }
    const std::vector<uint64_t> node_ids(nodeids, nodeids + num_nodes);
    auto report = sonata_report.get_report(report_name);
    report->record_data(step, node_ids);
    return 0;
}

int sonata_record_data(double step) {
    if (sonata_report.is_empty()) {
        return -3;
    }
    auto functor = std::mem_fn<void(double)>(&bbp::sonata::Report::record_data);
    sonata_report.apply_all(functor, step);
    return 0;
}

int sonata_check_and_flush(double timestep) {
    auto functor = std::mem_fn(&bbp::sonata::Report::check_and_flush);
    sonata_report.apply_all(functor, timestep);
    return 0;
}

int sonata_flush(double time) {
    if (sonata_report.is_empty()) {
        return -3;
    }
    auto functor = std::mem_fn(&bbp::sonata::Report::flush);
    sonata_report.apply_all(functor, time);
    return 0;
}

int sonata_set_max_buffer_size_hint(size_t buffer_size) {
    auto functor = std::mem_fn(&bbp::sonata::Report::set_max_buffer_size);
    sonata_report.apply_all(functor, buffer_size * 1048576);
    return 0;
}

int sonata_set_report_max_buffer_size_hint(const char* report_name, size_t buffer_size) {
    if (!sonata_report.report_exists(report_name)) {
        return -1;
    }
    auto report = sonata_report.get_report(report_name);
    report->set_max_buffer_size(buffer_size * 1048576);
    return 0;
}

void sonata_set_atomic_step(double step) {
    bbp::sonata::SonataReport::atomic_step_ = step;
}

int sonata_get_num_reports() {
    return sonata_report.get_num_reports();
}

void sonata_refresh_pointers(double* (*refresh_function)(double*) ) {
    std::function<double*(double*)> fun(refresh_function);  // This conversion is needed as
                                                            // apply_all is a magic template
    auto functor = std::mem_fn(&bbp::sonata::Report::refresh_pointers);
    sonata_report.apply_all(functor, fun);
}

void sonata_write_spikes(const char* population_name,
                         const double* timestamps,
                         uint64_t num_timestamps,
                         const int* node_ids,
                         uint64_t num_node_ids,
                         const char* output_dir) {
    const std::vector<double> spike_timestamps(timestamps, timestamps + num_timestamps);
    const std::vector<uint64_t> spike_node_ids(node_ids, node_ids + num_node_ids);
    const std::string output_directory(output_dir);
    const std::string population(population_name);
    bbp::sonata::SonataReport::write_spikes(output_directory, population, spike_timestamps, spike_node_ids);
}

// NOT REQUIRED FOR SONATA
int sonata_extra_mapping(const char*, uint64_t, int, const int*) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
void sonata_set_steps_to_buffer(int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
void sonata_set_auto_flush(int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
int sonata_time_data() {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
char* sonata_saveinit(const char*, int, const int*, const int*, int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return nullptr;
}
char* sonata_savebuffer(int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return nullptr;
}
void sonata_saveglobal() {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
void sonata_savestate(void) {
    logger->trace("Function NOT implemented");
}
char* sonata_restoreinit(const char*, const int*) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
char* sonata_restore(uint64_t, const int*, const int*) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
