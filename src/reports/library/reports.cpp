#include <iostream>
#include <bbp/sonata/reports.h>
#include <reports/library/sonatareport.hpp>
#include <reports/utils/logger.hpp>

using namespace bbp::sonata;

SonataReport sonata_report;

int sonata_clear() {
    sonata_report.clear();
    return 0;
}

int sonata_create_report(const char* report_name, double tstart, double tend, double dt, const char* kind) {
    try {
        if (!sonata_report.report_exists(report_name)) {
            sonata_report.create_report(report_name, kind, tstart, tend, dt);
        }
    } catch (const std::exception& err) {
        logger->error(err.what());
    }
    return 0;
}

int sonata_add_node(const char* report_name, uint64_t node_id) {
    try {
        if (sonata_report.report_exists(report_name)) {
            auto report = sonata_report.get_report(report_name);
            report->add_node(node_id);
        }
    } catch (const std::exception& err) {
        logger->error(err.what());
    }
    return 0;
}

int sonata_add_element(const char* report_name, uint64_t node_id, uint32_t element_id, double* voltage) {
    if (!sonata_report.report_exists(report_name)) {
        return 0;
    }
    try {
        auto report = sonata_report.get_report(report_name);
        auto node = report->get_node(node_id);
        node->add_element(voltage, element_id);
    } catch (const std::exception& err) {
        logger->error(err.what());
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
    SonataReport::m_min_steps_to_record = steps;
}

int sonata_record_node_data(double step, int num_nodes, int* nodeids, const char* report_name) {
    if (sonata_report.is_empty()) {
        return 0;
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
        return 0;
    }
    sonata_report.apply_all(&Report::record_data, step);
    return 0;
}

int sonata_end_iteration(double timestep) {
    sonata_report.apply_all(&Report::end_iteration, timestep);
    return 0;
}

int sonata_flush(double time) {
    if (sonata_report.is_empty()) {
        return 0;
    }
    sonata_report.apply_all(&Report::flush, time);
    return 0;
}

size_t sonata_set_max_buffer_size_hint(size_t buffer_size) {
    sonata_report.apply_all(&Report::set_max_buffer_size, buffer_size * /*1024*/1048576);
    return 0;
}

size_t sonata_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size) {
    if (!sonata_report.report_exists(report_name)) {
        return -1;
    }
    auto report = sonata_report.get_report(report_name);
    report->set_max_buffer_size(buffer_size * 1048576);
    return 0;
}

void sonata_set_atomic_step(double step) {
    SonataReport::m_atomic_step = step;
}

int sonata_get_num_reports() {
    return sonata_report.get_num_reports();
}

void sonata_refresh_pointers(double* (*refresh_function)(double*)) {
    std::function<double*(double*)> fun(refresh_function); // This conversion is needed as apply_all is a magic template
    sonata_report.apply_all(&Report::refresh_pointers, fun);
}

void sonata_write_spikes(const double* timestamps, uint64_t size_timestamps, const int* node_ids, uint64_t size_node_ids) {
    const std::vector<double> spike_timestamps(timestamps, timestamps + size_timestamps);
    const std::vector<int> spike_node_ids(node_ids, node_ids + size_node_ids);
    sonata_report.write_spikes(spike_timestamps, spike_node_ids);
}

// NOT REQUIRED FOR SONATA
int sonata_extra_mapping(char* report_name, uint64_t node_id, int num_values, int* values) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
void sonata_set_steps_to_buffer(int steps) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
void sonata_set_auto_flush(int mode) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
int sonata_time_data() {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
char* sonata_saveinit(char*, int, int*, int*, int) {
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
void sonata_savestate(void){
    logger->trace("Function NOT implemented");
}
char* sonata_restoreinit(char* save_file, int* length) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
char* sonata_restore(uint64_t node_id, int* piece_count, int* length) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
