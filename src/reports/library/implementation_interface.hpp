#pragma once
#include <algorithm>
#include <hdf5.h>
#include <iostream>
#include <numeric>
#include <set>
#include <tuple>
#include <vector>

#include "../utils/logger.h"
#include "sonatareport.h"

#if defined(HAVE_MPI)
#include <mpi.h>
#endif


namespace bbp {
namespace sonata {
namespace detail {

template <class TImpl>
struct Implementation {
    static int init(const std::vector<std::string>& report_names) {
        return TImpl::init(report_names);
    }
    static void close() {
        TImpl::close();
    }
    static std::tuple<hid_t, hid_t> prepare_write(const std::string& report_name, hid_t plist_id) {
        return TImpl::prepare_write(report_name, plist_id);
    }
    static hsize_t get_offset(const std::string& report_name, hsize_t value) {
        return TImpl::get_offset(report_name, value);
    }
    static hsize_t get_global_dims(const std::string& report_name, hsize_t value) {
        return TImpl::get_global_dims(report_name, value);
    }
    static void sort_spikes(std::vector<double>& spikevec_time,
                            std::vector<uint64_t>& spikevec_gid,
                            const std::string& order_by) {
        TImpl::sort_spikes(spikevec_time, spikevec_gid, order_by);
    }
};

static void local_spikevec_sort(std::vector<double>& isvect,
                                std::vector<uint64_t>& isvecg,
                                std::vector<double>& osvect,
                                std::vector<uint64_t>& osvecg,
                                const std::string& order_by) {
    osvect.resize(isvect.size());
    osvecg.resize(isvecg.size());
    // first build a permutation vector
    std::vector<uint64_t> perm(isvect.size());
    std::iota(perm.begin(), perm.end(), 0);

    if (order_by == "by_id") {
        // sort by time
        std::stable_sort(perm.begin(), perm.end(), [&](uint64_t i, uint64_t j) {
            return isvect[i] < isvect[j];
        });
        // then sort by gid
        std::stable_sort(perm.begin(), perm.end(), [&](uint64_t i, uint64_t j) {
            return isvecg[i] < isvecg[j];
        });
    } else if (order_by == "by_time") {
        // sort by gid (second predicate first)
        std::stable_sort(perm.begin(), perm.end(), [&](uint64_t i, uint64_t j) {
            return isvecg[i] < isvecg[j];
        });
        // then sort by time
        std::stable_sort(perm.begin(), perm.end(), [&](uint64_t i, uint64_t j) {
            return isvect[i] < isvect[j];
        });
    }

    // now apply permutation to time and gid output vectors
    std::transform(perm.begin(), perm.end(), osvect.begin(), [&](uint64_t i) { return isvect[i]; });
    std::transform(perm.begin(), perm.end(), osvecg.begin(), [&](uint64_t i) { return isvecg[i]; });
}

#if defined(HAVE_MPI)

static MPI_Comm get_Comm(const std::string& report_name) {
    if (SonataReport::communicators_.find(report_name) != SonataReport::communicators_.end()) {
        // Found
        return SonataReport::communicators_[report_name];
    }
    return MPI_COMM_WORLD;
}

struct ParallelImplementation {
    static int init(const std::vector<std::string>& report_names) {
        logger->info("Initializing PARALLEL implementation...");
        // size_t MPI type
        MPI_Datatype mpi_size_type = MPI_UINT64_T;
        if (sizeof(size_t) == 4) {
            mpi_size_type = MPI_UINT32_T;
        }

        int global_rank, global_size;
        MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &global_size);

        // Create a first communicator with the ranks with at least 1 report
        int num_reports = report_names.size();
        MPI_Comm_split(MPI_COMM_WORLD, num_reports == 0, 0, &SonataReport::has_nodes_);

        // Send report numbers and generate offset array for allgatherv
        std::vector<int> report_sizes(global_size);
        MPI_Allgather(
            &num_reports, 1, MPI_INT, report_sizes.data(), 1, MPI_INT, SonataReport::has_nodes_);
        std::vector<int> offsets(global_size + 1);
        offsets[0] = 0;
        std::partial_sum(report_sizes.begin(), report_sizes.end(), offsets.begin() + 1);

        // Create auxiliar map to associate hash with report name
        std::map<size_t, std::string> hash_report_names;
        std::hash<std::string> hasher;
        std::vector<size_t> local_report_hashes;
        local_report_hashes.reserve(num_reports);
        for (const auto& elem : report_names) {
            size_t report_hash = hasher(elem);
            local_report_hashes.push_back(report_hash);
            hash_report_names[report_hash] = elem;
        }

        // Get global number of reports
        size_t total_num_reports = std::accumulate(report_sizes.begin(), report_sizes.end(), 0);
        std::vector<size_t> global_report_hashes(total_num_reports);
        MPI_Allgatherv(local_report_hashes.data(),
                       num_reports,
                       mpi_size_type,
                       global_report_hashes.data(),
                       report_sizes.data(),
                       offsets.data(),
                       mpi_size_type,
                       SonataReport::has_nodes_);

        // Eliminate duplicates
        std::set<size_t> result(global_report_hashes.begin(), global_report_hashes.end());
        // Create communicators per report name
        for (auto& elem : global_report_hashes) {
            MPI_Comm_split(SonataReport::has_nodes_,
                           std::find(local_report_hashes.begin(),
                                     local_report_hashes.end(),
                                     elem) != local_report_hashes.end(),
                           0,
                           &SonataReport::communicators_[hash_report_names[elem]]);
        }

        /*logger->trace("WORLD RANK/SIZE: {}/{} \t Size communicators: {}", global_rank,
        global_size, SonataReport::communicators_.size()); for(auto& kv:
        SonataReport::communicators_) { int node_rank, node_size; MPI_Comm_rank(kv.second,
        &node_rank); MPI_Comm_size(kv.second, &node_size); logger->trace("WORLD RANK/SIZE: {}/{}
        \t report:{} \t RANK/SIZE: {}/{}", global_rank, global_size, kv.first, node_rank,
        node_size);
        }*/
        return global_rank;
    };

    static void close(){};
    static std::tuple<hid_t, hid_t> prepare_write(const std::string& report_name, hid_t plist_id) {
        // Enable MPI access
        MPI_Info info = MPI_INFO_NULL;
        H5Pset_fapl_mpio(plist_id, get_Comm(report_name), info);

        // Initialize independent/collective lists
        hid_t collective_list = H5Pcreate(H5P_DATASET_XFER);
        hid_t independent_list = H5Pcreate(H5P_DATASET_XFER);
        H5Pset_dxpl_mpio(collective_list, H5FD_MPIO_COLLECTIVE);
        H5Pset_dxpl_mpio(independent_list, H5FD_MPIO_INDEPENDENT);

        return std::make_tuple(collective_list, independent_list);
    };

    static hsize_t get_offset(const std::string& report_name, hsize_t value) {
        hsize_t offset = 0;
        MPI_Scan(&value, &offset, 1, MPI_UNSIGNED_LONG, MPI_SUM, get_Comm(report_name));
        offset -= value;
        return offset;
    };

    static hsize_t get_global_dims(const std::string& report_name, hsize_t value) {
        hsize_t global_dims = value;
        MPI_Allreduce(&value, &global_dims, 1, MPI_UNSIGNED_LONG, MPI_SUM, get_Comm(report_name));
        return global_dims;
    };

    static void sort_spikes(std::vector<double>& spikevec_time,
                            std::vector<uint64_t>& spikevec_gid,
                            const std::string& order_by) {
        double lmin_time = std::numeric_limits<double>::max();
        double lmax_time = std::numeric_limits<double>::min();
        if (!spikevec_time.empty()) {
            lmin_time = *(std::min_element(spikevec_time.begin(), spikevec_time.end()));
            lmax_time = *(std::max_element(spikevec_time.begin(), spikevec_time.end()));
        }

        double min_time;
        MPI_Allreduce(&lmin_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, SonataReport::has_nodes_);
        double max_time;
        MPI_Allreduce(&lmax_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, SonataReport::has_nodes_);

        std::vector<double> inTime = spikevec_time;
        std::vector<uint64_t> inGid = spikevec_gid;
        local_spikevec_sort(inTime, inGid, spikevec_time, spikevec_gid, order_by);

        int numprocs;
        MPI_Comm_size(SonataReport::has_nodes_, &numprocs);
        // allocate send and receive counts and displacements for MPI_Alltoallv
        std::vector<int> snd_cnts(numprocs);
        std::vector<int> rcv_cnts(numprocs);
        std::vector<int> snd_dsps(numprocs);
        std::vector<int> rcv_dsps(numprocs);

        double bin_t = (max_time - min_time) / numprocs;

        bin_t = bin_t ? bin_t : 1;
        // first find number of spikes in each time window
        for (const auto& st : spikevec_time) {
            int idx = (int) (st - min_time) / bin_t;
            snd_cnts[idx]++;
        }

        // now let each rank know how many spikes they will receive
        // and get in turn all the buffer sizes to receive
        MPI_Alltoall(&snd_cnts[0], 1, MPI_INT, &rcv_cnts[0], 1, MPI_INT, SonataReport::has_nodes_);
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
        std::vector<uint64_t> svg_buf(new_sz, 0);

        // now exchange data
        MPI_Alltoallv(spikevec_time.data(),
                      &snd_cnts[0],
                      &snd_dsps[0],
                      MPI_DOUBLE,
                      svt_buf.data(),
                      &rcv_cnts[0],
                      &rcv_dsps[0],
                      MPI_DOUBLE,
                      SonataReport::has_nodes_);
        MPI_Alltoallv(spikevec_gid.data(),
                      &snd_cnts[0],
                      &snd_dsps[0],
                      MPI_UINT64_T,
                      svg_buf.data(),
                      &rcv_cnts[0],
                      &rcv_dsps[0],
                      MPI_UINT64_T,
                      SonataReport::has_nodes_);

        local_spikevec_sort(svt_buf, svg_buf, spikevec_time, spikevec_gid, order_by);
    };
};

#else

struct SerialImplementation {
    static int init(const std::vector<std::string>& /*report_names*/) {
        logger->info("Initializing SERIAL implementation...");
        return 0;
    };
    static void close(){};
    static std::tuple<hid_t, hid_t> prepare_write(const std::string& /*report_name*/,
                                                  hid_t /*plist_id*/) {
        return std::make_tuple(H5Pcreate(H5P_DATASET_XFER), H5Pcreate(H5P_DATASET_XFER));
    }
    static hsize_t get_offset(const std::string& /*report_name*/, hsize_t /*value*/) {
        return 0;
    };
    static hsize_t get_global_dims(const std::string& /*report_name*/, hsize_t value) {
        return value;
    };
    static void sort_spikes(std::vector<double>& spikevec_time,
                            std::vector<uint64_t>& spikevec_gid,
                            const std::string& order_by) {
        std::vector<double> times(spikevec_time);
        std::vector<uint64_t> gids(spikevec_gid);
        local_spikevec_sort(times, gids, spikevec_time, spikevec_gid, order_by);
    };
};

#endif

}  // namespace detail
}  // namespace sonata
}  // namespace bbp

using Implementation = bbp::sonata::detail::Implementation<
#if defined(HAVE_MPI)
    bbp::sonata::detail::ParallelImplementation
#else
    bbp::sonata::detail::SerialImplementation
#endif
    >;
