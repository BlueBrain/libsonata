#include <iostream>
#include <bbp/reports/records.h>

#ifdef DEBUG
#include <iostream>
#endif

ReportingLib InitReports;

int records_clear() {
    InitReports.clear();
}

int records_add_report(const char* reportName, uint64_t node_id, uint64_t gid, uint64_t vgid,
                       double tstart, double tend, double dt, const char* kind) {
    // if we are adding reports, then we should reset the flag for sharing reports
    ReportingLib::first_report = true;

    return InitReports.add_report(std::string(reportName), node_id, gid, vgid, tstart, tend, dt, std::string(kind));
}

int records_add_var_with_mapping(const char* report_name, uint64_t node_id, double* voltage) {

    return InitReports.add_variable(std::string(report_name), node_id, voltage);
}

void records_setup_communicator() {
    InitReports.make_global_communicator();
}

int records_finish_and_share() {
    if (InitReports.is_empty()) {
        return 0;
    }
    int rank, num_nodes = 0;
    if(ReportingLib::first_report) {
        InitReports.share_and_prepare();
        ReportingLib::first_report = false;
    }
    return 0;
}

int records_nrec(double time, int num_nodes, uint64_t* nodeids, const char* report_name) {
    if (!InitReports.is_empty()) {
        const std::vector<uint64_t> node_ids(nodeids, nodeids + num_nodes);
        return InitReports.record_data(time, node_ids, std::string(report_name));
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
    return InitReports.set_max_buffer_size(buffer_size * 1024/*1048576*/);
}

size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size) {
    return InitReports.set_max_buffer_size(std::string(report_name), buffer_size * 1024/*1048576*/);
}

void records_set_atomic_step(double step) {
    ReportingLib::m_atomic_step = step;
}

int records_get_num_reports() {
    return InitReports.get_num_reports();
}
