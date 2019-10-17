#include <iostream>
#include <bbp/reports/records.h>
#include <reports/utils/logger.hpp>

ReportingLib InitReports;

int records_clear() {
    InitReports.clear();
    return 0;
}

int records_add_report(const char* reportName, uint64_t node_id, uint64_t gid, uint64_t vgid,
                       double tstart, double tend, double dt, int mapping_size, const char* kind,
                       int attributes_mapping_size, char* unit) {
    // if we are adding reports, then we should reset the flag for sharing reports
    ReportingLib::first_report = true;

    return InitReports.add_report(std::string(reportName), node_id, gid, vgid, tstart, tend, dt, std::string(kind));
}

int records_add_var_with_mapping(const char* report_name, uint64_t node_id, double* voltage, int mapping_size, int* mapping_value) {
    uint32_t element_id = mapping_value[0];
    return InitReports.add_variable(std::string(report_name), node_id, voltage, element_id);
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
    if (!InitReports.is_empty()) {
        const std::vector<uint64_t> node_ids(nodeids, nodeids + num_nodes);
        return InitReports.record_nodes_data(step, node_ids, std::string(report_name));
    }
    return 0;
}

int records_rec(double step) {
    if (!InitReports.is_empty()) {
        return InitReports.record_data(step);
    }
    return 0;
}

int records_end_iteration (double timestep) {
    return InitReports.end_iteration(timestep);
}

int records_flush(double time) {
    if (!InitReports.is_empty())
        return InitReports.flush(time);
    return 0;
}

size_t records_set_max_buffer_size_hint(size_t buffer_size) {
    return InitReports.set_max_buffer_size(buffer_size * /*1024*/1048576);
}

size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size) {
    return InitReports.set_max_buffer_size(std::string(report_name), buffer_size * /*1024*/1048576);
}

void records_set_atomic_step(double step) {
    ReportingLib::m_atomic_step = step;
}

int records_get_num_reports() {
    return InitReports.get_num_reports();
}

void records_refresh_pointers(double* (*refresh_function)(double*)) {
    InitReports.refresh_pointers(refresh_function);
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
