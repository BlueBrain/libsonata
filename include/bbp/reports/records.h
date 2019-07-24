#pragma once

#include <reports/library/reportinglib.hpp>

/**
 * Provides a bridge between c-based programs in order to have access to the c++-based objects
 */
#ifndef __cplusplus
#include <stddef.h>
#endif
#if defined(__cplusplus)
#include <cstddef>
extern "C" {
#endif
int records_clear();
/**
 * Add cell to existing or new report
 */
int records_add_report(const char* reportName,
                        int cellnumber,
                        unsigned long gid,
                        unsigned long vgid,
                        double tstart,
                        double tend,
                        double dt,
                        const char* kind);

/**
 * Add compartment/spike to an existing cell on a report
 */
int records_add_var_with_mapping(const char* _reportname,
                                 int _cellnumber,
                                 double* _pointer);
/**
 * Setup buffers
 */
int records_finish_and_share();

void records_setup_communicator();

/**
 * Save data of cellids[] to buffer
 */
int records_nrec(double _time, int ncells, int* cellids, const char* report_name);

int records_end_iteration (double timestep);

int records_getNumReports();

int records_flush(double time);

/**
 * Set a suggested maximum memory size each individual reports can use as a buffer
 * @param buf_size requested maximum memory allocatable by a Report buffer in Mbytes
 * @return 0
 */
size_t records_set_max_buffer_size_hint(size_t buf_size);

/**
 * Set a suggested maximum memory size each individual reports can use as a buffer
 * @param buf_size requested maximum memory allocatable by a Report buffer in Mbytes
 * @return 0
 */
size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buf_size);

void records_set_atomic_step(double step);

#if defined(__cplusplus)
}
#endif
