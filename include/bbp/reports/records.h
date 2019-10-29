#pragma once

/**
 * Provides a bridge between c-based programs in order to have access to the c++-based objects
 */
#include <vector>
#ifndef __cplusplus
#include <stddef.h>
#endif
#if defined(__cplusplus)
#include <cstddef>
extern "C" {
#endif


/*! \brief Add node to existing or new report
 * @param report_name name of the report to be created or added node to
 * @param gid node/cell identifier
 * @param tstart start time of recording for the report
 * @param tend stop time of recording for the report
 * @param dt frequency of recordings
 */
int records_add_report(const char* report_name,
                        uint64_t node_id,
                        uint64_t gid,
                        uint64_t vgid,
                        double tstart,
                        double tend,
                        double dt,
                        int mapping_size,
                        const char* kind,
                        int attributes_mapping_size,
                        char* unit);

/**
 * \brief Add compartment values to an existing node on a report
 */
int records_add_var_with_mapping(const char* report_name,
                                 uint64_t node_id,
                                 double* voltage,
                                 int mapping_size,
                                 int* mapping_value);
/**
 * \brief Setup buffers and create datasets
 */
int records_finish_and_share();

/**
 * \brief Initialize communicators
 */
void records_setup_communicator();

/**
 * \brief Minimum number steps needed to allocate in a single buffer
 */
void records_set_min_steps_to_record(int steps);

/**
 * \brief Spike arrays to be written to file
 */
void records_write_spikes(const std::vector<double>& spike_timestamps, const std::vector<int>& spike_node_ids);

/**
 * \brief Save data of nodeids[] to buffer
 */
int records_nrec(double step, int num_nodes, int* nodeids, const char* report_name);

/**
 * \brief Save data of all the nodes to buffer
 */
int records_rec(double step);

/**
 * \brief Check status of the recordings/buffers and flush if necessary
 */
int records_end_iteration(double timestep);

int records_get_num_reports();

/**
 * \brief Flush buffers to file
 */
int records_flush(double time);

void records_refresh_pointers(double* (*refresh_function)(double*));
/**
 * \brief Set a suggested maximum memory size each individual report can use as a buffer
 * @param buffer_size requested maximum memory allocatable by a Report buffer in MBytes
 * @return 0
 */
size_t records_set_max_buffer_size_hint(size_t buffer_size);

/**
 * \brief Set a suggested maximum memory size that a specific report can use as a buffer
 * @param report_name name of the report to apply maximum buffer size
 * @param buffer_size requested maximum memory allocatable by a Report buffer in MBytes
 * @return 0
 */
size_t records_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size);

/*! \brief Clear all the reports
*/
int records_clear();

void records_set_atomic_step(double step);

// NOT REQUIRED FOR SONATA
int records_extra_mapping(char* report_name, uint64_t node_id, int num_values, int* values);
void records_set_steps_to_buffer(int steps);
void records_set_auto_flush(int mode);
int records_time_data();
char* records_saveinit(char*, int, int*, int*, int);
char* records_savebuffer(int);
void records_saveglobal();
void records_savestate(void);
char* records_restoreinit(char* save_file, int* length);
char* records_restore(uint64_t node_id, int* piece_count, int* length);

#if defined(__cplusplus)
}
#endif
