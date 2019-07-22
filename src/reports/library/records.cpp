#include <bbp/reports/records.hpp>
#include <iostream>

#ifdef DEBUG
// Only with debug
#include <iostream>
#endif

ReportingLib InitReports;
bool first_report = true;

int records_clear() {
    InitReports.clear();
}

int records_add_report(const char* reportName, int cellnumber, unsigned long gid, unsigned long vgid,
                       double tstart, double tend, double dt,const char* kind) {
    // if we are adding reports, then we should reset the flag for sharing reports
    first_report = 1;

    return InitReports.add_report(std::string(reportName), cellnumber, gid, vgid, tstart, tend, dt, std::string(kind));
}

int records_add_var_with_mapping(const char* reportname, int cellnumber, double* pointer) {

    return InitReports.add_variable(std::string(reportname), cellnumber, pointer);
}

void records_setup_communicator() {
    InitReports.make_global_communicator();
}

int records_finish_and_share() {
    if (InitReports.is_empty()) {
        return 0;
    }
    int rank, num_nodes = 0;
    if(first_report) {
        InitReports.share_and_prepare();
        first_report = false;
    }
    return 0;
}

int records_nrec(double _time, int ncells, int* cellids, const char* report_name) {
    if (!InitReports.is_empty())
        return InitReports.record_data(_time, ncells, cellids, std::string(report_name));
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

size_t records_set_max_buffer_size_hint(size_t buf_size) {
    return InitReports.set_max_buffer_size(buf_size * 1024/*1048576*/);
}

size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buf_size) {
    return InitReports.set_max_buffer_size(std::string(report_name), buf_size * 1024/*1048576*/);
}

void records_set_atomic_step(double step) {
    ReportingLib::m_atomicStep = step;
}

int records_getNumReports() {
    return InitReports.get_num_reports();
}
