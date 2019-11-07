#include <iostream>
#include <bbp/reports/records.h>
#include <reports/library/reportinglib.hpp>
#include <reports/utils/logger.hpp>

ReportingLib InitReports;

int records_clear() {
    InitReports.clear();
    return 0;
}

int records_add_report(const char* reportName, uint64_t node_id, uint64_t gid, uint64_t /* vgid */,
                       double tstart, double tend, double dt, int mapping_size, const char* kind,
                       int attributes_mapping_size, char* unit) {
    // if we are adding reports, then we should reset the flag for sharing reports
    ReportingLib::first_report = true;

    try {
        if (!InitReports.report_exists(reportName)) {
            InitReports.create_report(reportName, kind, tstart, tend, dt);
        }
        auto report = InitReports.get_report(reportName);
        report->add_node(node_id, gid);
    } catch (const std::exception& err) {
        logger->error(err.what());
    }

    return 0;
}

int records_add_var_with_mapping(const char* report_name, uint64_t node_id, double* voltage, int mapping_size, int* mapping_value) {
    if (!InitReports.report_exists(report_name)) {
        return 0;
    }

    uint32_t element_id = mapping_value[0];
    try {
        auto report = InitReports.get_report(report_name);
        auto node = report->get_node(node_id);
        node->add_element(voltage, element_id);
    } catch (const std::exception& err) {
        logger->error(err.what());
    }
    return 0;
}

void records_setup_communicator() {
    InitReports.make_global_communicator();
}

int records_finish_and_share() {
    if(ReportingLib::first_report) {
        InitReports.prepare_datasets();
        ReportingLib::first_report = false;
    }
    return 0;
}

void records_set_min_steps_to_record(int steps) {
    ReportingLib::m_min_steps_to_record = steps;
}

int records_nrec(double step, int num_nodes, int* nodeids, const char* report_name) {
    if (InitReports.is_empty()) {
        return 0;
    }

    if (!InitReports.report_exists(report_name)) {
        return -1;
    }

    const std::vector<uint64_t> node_ids(nodeids, nodeids + num_nodes);
    auto report = InitReports.get_report(report_name);
    report->record_data(step, node_ids);

    return 0;
}

int records_rec(double step) {
    if (InitReports.is_empty()) {
        return 0;
    }

    InitReports.apply_all(&Report::record_data, step);

    return 0;
}

int records_end_iteration(double timestep) {
    InitReports.apply_all(&Report::end_iteration, timestep);

    return 0;
}

int records_flush(double time) {
    if (InitReports.is_empty()) {
        return 0;
    }

    InitReports.apply_all(&Report::flush, time);

    return 0;
}

size_t records_set_max_buffer_size_hint(size_t buffer_size) {
    InitReports.apply_all(&Report::set_max_buffer_size, buffer_size * /*1024*/1048576);
    return 0;
}

size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size) {
    if (!InitReports.report_exists(report_name)) {
        return -1;
    }
    auto report = InitReports.get_report(report_name);
    report->set_max_buffer_size(buffer_size * 1048576);

    return 0;
}

void records_set_atomic_step(double step) {
    ReportingLib::m_atomic_step = step;
}

int records_get_num_reports() {
    return InitReports.get_num_reports();
}

void records_refresh_pointers(double* (*refresh_function)(double*)) {
    std::function<double*(double*)> fun(refresh_function); // This conversion is needed as apply_all is a magic template
    InitReports.apply_all(&Report::refresh_pointers, fun);
}

void records_write_spikes(const std::vector<double>& spike_timestamps, const std::vector<int>& spike_node_ids) {
    InitReports.write_spikes(spike_timestamps, spike_node_ids);
}

// NOT REQUIRED FOR SONATA
int records_extra_mapping(char* report_name, uint64_t node_id, int num_values, int* values) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
void records_set_steps_to_buffer(int steps) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
void records_set_auto_flush(int mode) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
int records_time_data() {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return 0;
}
char* records_saveinit(char*, int, int*, int*, int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return nullptr;
}
char* records_savebuffer(int) {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
    return nullptr;
}
void records_saveglobal() {
    logger->trace("Function {} NOT implemented", __FUNCTION__);
}
void records_savestate(void){
    logger->trace("Function NOT implemented");
}
char* records_restoreinit(char* save_file, int* length) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
char* records_restore(uint64_t node_id, int* piece_count, int* length) {
    logger->trace("Function NOT implemented");
    return nullptr;
}
