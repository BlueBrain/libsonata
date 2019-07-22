#include <stdlib.h>
#include <new>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <numeric>

#include "reportinglib.hpp"
#include "sonata_format.hpp"

SonataFormat::SonataFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<cells_t> cells)
: ReportFormat(report_name, max_buffer_size, num_steps, cells) {
    index_pointers.resize(cells->size());
    m_ioWriter = IoWriter::create_IoWriter(HDF5, report_name);
}

SonataFormat::~SonataFormat() {}

void SonataFormat::prepare_dataset() {

    std::cout << "Preparing SonataFormat Dataset for report: " << m_reportName << std::endl;

    // Prepare /report and /spikes headers
   for(auto& kv: *m_cells) {
        // /report
        const std::vector<uint32_t > compartment_ids = kv.second.get_compartment_ids();
        element_ids.insert(element_ids.end(), compartment_ids.begin(), compartment_ids.end());
        node_ids.push_back(kv.first);

        // /spikes
        const std::vector<double*> spikes = kv.second.get_spike_timestamps();
        for(auto& timestamp: spikes) {

            spike_node_ids.push_back(kv.first);
            spike_timestamps.push_back(*timestamp);
        }
    }

     int compartment_offset = get_compartment_offset();

    std::cout << "Total compartments are: " << m_totalCompartments << " and compartment_offset is: " << compartment_offset << std::endl;

    // Prepare index pointers
    if(!index_pointers.empty()) {
        index_pointers[0] = compartment_offset;
    }
    for (int i = 1; i < index_pointers.size(); i++) {
        int previousGid = node_ids[i-1];
        index_pointers[i] = index_pointers[i-1] + m_cells->at(previousGid).get_num_compartments();
    }

    // We only write the headers if there are compartments/spikes to write
    if(m_totalCompartments > 0 ) {
        write_report_header();
    }

    if(m_totalSpikes > 0) {
        write_spikes_header();
    }
}
void SonataFormat::write_report_header() {

    std::cout << "Writing report header!" << std::endl;
    m_ioWriter->configure_group("/report");
    m_ioWriter->configure_group("/report/mapping");
    m_ioWriter->configure_dataset("/report/data", m_numSteps, m_totalCompartments);

    m_ioWriter->write("/report/mapping/node_ids", node_ids);
    m_ioWriter->write("/report/mapping/index_pointers", index_pointers);
    m_ioWriter->write("/report/mapping/element_ids", element_ids);
}

void SonataFormat::write_spikes_header() {

    std::cout << "Writing spike header!" << std::endl;
    m_ioWriter->configure_group("/spikes");
    m_ioWriter->configure_attribute("/spikes", "sorting");
#ifdef HAVE_MPI
    sort_spikes(spike_timestamps, spike_node_ids);
#endif
    m_ioWriter->write("/spikes/timestamps", spike_timestamps);
    m_ioWriter->write("/spikes/node_ids", spike_node_ids);
}

void SonataFormat::write_data() {

    if(m_remainingSteps > 0) {
        std::cout << "Writing data from sonataFormat! " << std::endl;
        std::cout << "++Remaining steps: " << m_remainingSteps << std::endl;
        std::cout << "++Steps to write: " << m_steps_to_write << std::endl;
        std::cout << "++total compartments: " << m_totalCompartments << std::endl;
        if (m_remainingSteps < m_steps_to_write) {
            // Write remaining steps
            m_ioWriter->write("Data", m_reportBuffer, m_remainingSteps, m_numSteps, m_totalCompartments);
        } else {
            m_ioWriter->write("Data", m_reportBuffer, m_steps_to_write, m_numSteps, m_totalCompartments);
        }
    }
}

void SonataFormat::close() {
    m_ioWriter->close();
}

int SonataFormat::get_compartment_offset() {
    int compartment_offset = 0;
#ifdef HAVE_MPI
    MPI_Scan(&m_totalCompartments, &compartment_offset, 1, MPI_INT, MPI_SUM, ReportingLib::m_allCells);
    compartment_offset -= m_totalCompartments;
#endif
    return compartment_offset;
}

#ifdef HAVE_MPI
// Sort spikes
void SonataFormat::sort_spikes(std::vector<double>& spikevec_time, std::vector<int>& spikevec_gid) {
    int numprocs;
    MPI_Comm_size(ReportingLib::m_allCells, &numprocs);
    double lmin_time = *(std::min_element(spikevec_time.begin(), spikevec_time.end()));
    double lmax_time = *(std::max_element(spikevec_time.begin(), spikevec_time.end()));
    double min_time = nrnmpi_dbl_allmin(lmin_time, numprocs);
    double max_time = nrnmpi_dbl_allmax(lmax_time, numprocs);

    std::vector<double> inTime = spikevec_time;
    std::vector<int> inGid = spikevec_gid;
    local_spikevec_sort(inTime, inGid, spikevec_time, spikevec_gid);

    // allocate send and receive counts and displacements for MPI_Alltoallv
    std::vector<int> snd_cnts(numprocs);
    std::vector<int> rcv_cnts(numprocs);
    std::vector<int> snd_dsps(numprocs);
    std::vector<int> rcv_dsps(numprocs);

    double bin_t = (max_time - min_time) / numprocs;

    bin_t = bin_t ? bin_t : 1;
    // first find number of spikes in each time window
    for (const auto& st : spikevec_time) {
        int idx = (int)(st - min_time) / bin_t;
        snd_cnts[idx]++;
    }

    // now let each rank know how many spikes they will receive
    // and get in turn all the buffer sizes to receive
    nrnmpi_int_alltoall(&snd_cnts[0], &rcv_cnts[0], 1);
    for (int i = 1; i < numprocs; i++) {
        rcv_dsps[i] = rcv_dsps[i - 1] + rcv_cnts[i - 1];
    }

    for (int i = 1; i < numprocs; i++) {
        snd_dsps[i] = snd_dsps[i - 1] + snd_cnts[i - 1];
    }

    std::size_t new_sz = 0;
    for (const auto& r : rcv_cnts) {
        new_sz += r;
    }

    // prepare new sorted vectors
    std::vector<double> svt_buf(new_sz, 0.0);
    std::vector<int> svg_buf(new_sz, 0);

    // now exchange data
    nrnmpi_dbl_alltoallv(spikevec_time.data(), &snd_cnts[0], &snd_dsps[0], svt_buf.data(),
                         &rcv_cnts[0], &rcv_dsps[0]);
    nrnmpi_int_alltoallv(spikevec_gid.data(), &snd_cnts[0], &snd_dsps[0], svg_buf.data(),
                         &rcv_cnts[0], &rcv_dsps[0]);

    local_spikevec_sort(svt_buf, svg_buf, spikevec_time, spikevec_gid);
}

double SonataFormat::nrnmpi_dbl_allmin(double x, int numprocs) {
    double result;
    if (numprocs < 2) {
        return x;
    }
    MPI_Allreduce(&x, &result, 1, MPI_DOUBLE, MPI_MIN, ReportingLib::m_allCells);
    return result;
}

double SonataFormat::nrnmpi_dbl_allmax(double x, int numprocs) {
    double result;
    if (numprocs < 2) {
        return x;
    }
    MPI_Allreduce(&x, &result, 1, MPI_DOUBLE, MPI_MAX, ReportingLib::m_allCells);
    return result;
}

void SonataFormat::nrnmpi_int_alltoall(int* s, int* r, int n) {
    MPI_Alltoall(s, n, MPI_INT, r, n, MPI_INT, ReportingLib::m_allCells);
}

void SonataFormat::nrnmpi_dbl_alltoallv(double* s, int* scnt,int* sdispl,
                                        double* r, int* rcnt, int* rdispl) {
    MPI_Alltoallv(s, scnt, sdispl, MPI_DOUBLE, r, rcnt, rdispl, MPI_DOUBLE, ReportingLib::m_allCells);
}

void SonataFormat::nrnmpi_int_alltoallv(int* s, int* scnt, int* sdispl, int* r, int* rcnt, int* rdispl) {
    MPI_Alltoallv(s, scnt, sdispl, MPI_INT, r, rcnt, rdispl, MPI_INT, ReportingLib::m_allCells);
}

void SonataFormat::local_spikevec_sort(std::vector<double>& isvect,
                                       std::vector<int>& isvecg,
                                       std::vector<double>& osvect,
                                       std::vector<int>& osvecg) {
    osvect.resize(isvect.size());
    osvecg.resize(isvecg.size());
    // first build a permutation vector
    std::vector<std::size_t> perm(isvect.size());
    std::iota(perm.begin(), perm.end(), 0);
    // sort by gid (second predicate first)
    std::stable_sort(perm.begin(), perm.end(),
                     [&](std::size_t i, std::size_t j) { return isvecg[i] < isvecg[j]; });
    // then sort by time
    std::stable_sort(perm.begin(), perm.end(),
                     [&](std::size_t i, std::size_t j) { return isvect[i] < isvect[j]; });
    // now apply permutation to time and gid output vectors
    std::transform(perm.begin(), perm.end(), osvect.begin(),
                   [&](std::size_t i) { return isvect[i]; });
    std::transform(perm.begin(), perm.end(), osvecg.begin(),
                   [&](std::size_t i) { return isvecg[i]; });
}
#endif