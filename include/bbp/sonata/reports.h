#pragma once

/**
 * Provides a bridge between c-based programs in order to have access to the c++-based objects
 */
#ifndef __cplusplus
#include <stddef.h>
#endif
#if defined(__cplusplus)
#include <cstddef>
#include <stdint.h>
extern "C" {
#endif


/*! \brief Add node to existing or new report
 * @param report_name name of the report to be created or added node to
 * @param gid node/cell identifier
 * @param tstart start time of recording for the report
 * @param tend stop time of recording for the report
 * @param dt frequency of recordings
 * @param kind type of report (soma, compartment)
 * @return 0 if operator succeeded, -1 otherwise
 */
int sonata_create_report(
    const char* report_name, double _tstart, double _tend, double _dt, const char* kind);

/*!
 * \brief Add a node to an existing report
 * \param report_name name of the report
 * \param node_id node identifier
 * \return 0 if operator succeeded, -2 if the report doesn't exist, -1 for other errors.
 */
int sonata_add_node(const char* report_name, uint64_t node_id);

/**
 * \brief Add compartment values to an existing node on a report
 * \return 0 if operator succeeded, -2 if the report doesn't exist, -3 if the specified node
 * doesn't exist, -1 for other errors.
 */
int sonata_add_element(const char* report_name,
                       uint64_t node_id,
                       uint32_t element_id,
                       double* voltage);
/**
 * \brief Setup buffers and create datasets
 * \return 0
 */
int sonata_prepare_datasets();

/**
 * \brief Initialize communicators
 */
void sonata_setup_communicators();

/**
 * \brief Minimum number steps needed to allocate in a single buffer
 */
void sonata_set_min_steps_to_record(int steps);

/**
 * \brief Spike arrays to be written to file
 */
void sonata_write_spikes(const double* spike_timestamps,
                         uint64_t num_timestamps,
                         const int* spike_node_ids,
                         uint64_t num_node_ids,
                         const char* output_dir);

/**
 * \brief Save data of nodeids[] to buffer
 * \return -3 if the Sonata report doesn't exist, -1 if the report name doesn't exist, 0 otherwise
 */
int sonata_record_node_data(double step, int num_nodes, int* nodeids, const char* report_name);

/**
 * \brief Save data of all the nodes to buffer
 * \return -3 if the Sonata report doesn't exist, 0 otherwise
 */
int sonata_record_data(double step);

/**
 * \brief Check status of the recordings/buffers and flush if necessary
 * \return 0
 */
int sonata_end_iteration(double timestep);

/**
 * \return number of reports
 */
int sonata_get_num_reports();

/**
 * \brief Flush buffers to file
 * \return -3 if there was nothing to flush, 0 otherwise
 */
int sonata_flush(double time);

void sonata_refresh_pointers(double* (*refresh_function)(double*) );
/**
 * \brief Set a suggested maximum memory size each individual report can use as a buffer
 * @param buffer_size requested maximum memory allocatable by a Report buffer in MBytes
 * @return 0
 */
int sonata_set_max_buffer_size_hint(size_t buffer_size);

/**
 * \brief Set a suggested maximum memory size that a specific report can use as a buffer
 * @param report_name name of the report to apply maximum buffer size
 * @param buffer_size requested maximum memory allocatable by a Report buffer in MBytes
 * @return -1 if the Sonata report doesn't exist, 0 otherwise
 */
int sonata_set_report_max_buffer_size_hint(char* report_name, size_t buffer_size);

/*! \brief Clear all the reports
 * \return 0
 */
int sonata_clear();

void sonata_set_atomic_step(double step);

// NOT REQUIRED FOR SONATA
int sonata_extra_mapping(char* report_name, uint64_t node_id, int num_values, int* values);
void sonata_set_steps_to_buffer(int steps);
void sonata_set_auto_flush(int mode);
int sonata_time_data();
char* sonata_saveinit(char*, int, int*, int*, int);
char* sonata_savebuffer(int);
void sonata_saveglobal();
void sonata_savestate(void);
char* sonata_restoreinit(char* save_file, int* length);
char* sonata_restore(uint64_t node_id, int* piece_count, int* length);

#if defined(__cplusplus)
}
#endif
