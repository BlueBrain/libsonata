/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include "hdf5_mutex.hpp"

#include <bbp/sonata/population.h>

#include <algorithm>  // stable_sort, transform
#include <iterator>   // back_inserter
#include <numeric>    // iota
#include <vector>

#include <fmt/format.h>

#include <highfive/H5File.hpp>

namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

namespace {

const char* H5_DYNAMICS_PARAMS = "dynamics_params";
const char* H5_LIBRARY = "@library";


std::set<std::string> _listChildren(const HighFive::Group& group,
                                    const std::set<std::string>& ignoreNames = {}) {
    std::set<std::string> result;
    for (const auto& name : group.listObjectNames()) {
        if (ignoreNames.count(name)) {
            continue;
        }
        result.insert(name);
    }
    return result;
}

std::set<std::string> _listExplicitEnumerations(const HighFive::Group h5Group,
                                                const std::set<std::string>& attrs) {
    std::set<std::string> names{};
    const std::set<std::string> enums(_listChildren(h5Group));
    std::set_intersection(enums.begin(),
                          enums.end(),
                          attrs.begin(),
                          attrs.end(),
                          std::inserter(names, names.begin()));
    return names;
}


template <typename T>
std::vector<T> _readChunk(const HighFive::DataSet& dset, const Selection::Range& range) {
    std::vector<T> result;
    assert(range.first < range.second);
    auto chunkSize = static_cast<size_t>(range.second - range.first);
    dset.select({static_cast<size_t>(range.first)}, {chunkSize}).read(result);
    return result;
}


template <typename T, typename std::enable_if<!std::is_pod<T>::value>::type* = nullptr>
std::vector<T> _readSelection(const HighFive::DataSet& dset, const Selection& selection) {
    if (selection.ranges().size() == 1) {
        return _readChunk<T>(dset, selection.ranges().front());
    }

    std::vector<T> result;

    // for POD types we can pre-allocate result vector... see below template specialization
    for (const auto& range : selection.ranges()) {
        for (auto& x : _readChunk<T>(dset, range)) {
            result.emplace_back(std::move(x));
        }
    }

    return result;
}


template <typename T, typename std::enable_if<std::is_pod<T>::value>::type* = nullptr>
std::vector<T> _readSelection(const HighFive::DataSet& dset, const Selection& selection) {
    if (selection.ranges().empty()) {
        return {};
    } else if (selection.ranges().size() == 1) {
        return _readChunk<T>(dset, selection.ranges().front());
    }

    const auto ids = selection.flatten();

    // TODO Rewrite to not use IDs but ID ranges.
    std::vector<std::size_t> ids_index(ids.size());
    std::iota(ids_index.begin(), ids_index.end(), std::size_t(0));
    std::stable_sort(ids_index.begin(), ids_index.end(), [&ids](size_t i0, size_t i1) {
        return ids[i0] < ids[i1];
    });

    std::vector<std::size_t> ids_sorted;
    ids_sorted.reserve(ids.size());
    std::transform(ids_index.begin(),
                   ids_index.end(),
                   std::back_inserter(ids_sorted),
                   [&ids](size_t i) { return static_cast<size_t>(ids[i]); });

    std::vector<T> linear_result;
    dset.select(HighFive::ElementSet{ids_sorted}).read(linear_result);

    std::vector<T> result(ids_sorted.size());
    for (size_t i = 0; i < ids_sorted.size(); ++i) {
        result[ids_index[i]] = linear_result[i];
    }

    return result;
}

namespace collective {
template <typename T>
std::vector<T> _readSelection(const HighFive::DataSet& dset,
                              const Selection& selection,
                              const DataTransferOpts& data_transfer_opts) {
    // TODO Guard existence of MPI, and fall back to regular `_readSelection`.
    const auto ids = selection.flatten();

    // TODO optimize if canonical.
    // TODO get the `std::move` twist back for canonical selections

    // TODO Rewrite to not use IDs but ID ranges.
    std::vector<std::size_t> ids_index(ids.size());
    std::iota(ids_index.begin(), ids_index.end(), std::size_t(0));
    std::stable_sort(ids_index.begin(), ids_index.end(), [&ids](size_t i0, size_t i1) {
        return ids[i0] < ids[i1];
    });

    std::vector<std::size_t> ids_sorted;
    ids_sorted.reserve(ids.size());
    std::transform(ids_index.begin(),
                   ids_index.end(),
                   std::back_inserter(ids_sorted),
                   [&ids](size_t i) { return static_cast<size_t>(ids[i]); });

    std::vector<T> linear_result;

    auto xfer_props = HighFive::DataTransferProps{};
    data_transfer_opts.apply(xfer_props);

    dset.select(HighFive::ElementSet{ids_sorted}).read(linear_result, xfer_props);

    std::vector<T> result(ids_sorted.size());
    for (size_t i = 0; i < ids_sorted.size(); ++i) {
        result[ids_index[i]] = linear_result[i];
    }

    return result;
}
}  // namespace collective


template <typename T>
std::vector<T> _readSelection(const HighFive::DataSet& dset,
                              const Selection& selection,
                              const DataTransferOpts& data_transfer_opts) {
    return collective::_readSelection<T>(dset, selection, data_transfer_opts);
}

}  // unnamed namespace


inline HighFive::File open_hdf5_file(const std::string& filename,
                                     const FileAccessOpts& file_access_opts) {
    HighFive::FileAccessProps fapl;
    file_access_opts.apply(fapl);
    return HighFive::File(filename, HighFive::File::ReadOnly, fapl);
}

struct Population::Impl {
    Impl(const std::string& h5FilePath,
         const std::string&,
         const std::string& _name,
         const std::string& _prefix,
         const IoOpts& io_opts)
        : name(_name)
        , prefix(_prefix)
        , h5File(open_hdf5_file(h5FilePath, io_opts))
        , h5Root(h5File.getGroup(fmt::format("/{}s", prefix)).getGroup(name))
        , attributeNames(_listChildren(h5Root.getGroup("0"), {H5_DYNAMICS_PARAMS, H5_LIBRARY}))
        , attributeEnumNames(
              h5Root.getGroup("0").exist(H5_LIBRARY)
                  ? _listExplicitEnumerations(h5Root.getGroup("0").getGroup(H5_LIBRARY),
                                              attributeNames)
                  : std::set<std::string>{})
        , dynamicsAttributeNames(
              h5Root.getGroup("0").exist(H5_DYNAMICS_PARAMS)
                  ? _listChildren(h5Root.getGroup("0").getGroup(H5_DYNAMICS_PARAMS))
                  : std::set<std::string>{})
        , io_opts(io_opts) {
        if (h5Root.exist("1")) {
            throw SonataError("Only single-group populations are supported at the moment");
        }
    }

    HighFive::DataSet getAttributeDataSet(const std::string& name) const {
        if (!attributeNames.count(name)) {
            throw SonataError(fmt::format("No such attribute: '{}'", name));
        }
        return h5Root.getGroup("0").getDataSet(name);
    }

    HighFive::DataSet getLibraryDataSet(const std::string& name) const {
        if (!attributeEnumNames.count(name)) {
            throw SonataError(fmt::format("No such enumeration attribute: '{}'", name));
        }
        return h5Root.getGroup("0").getGroup(H5_LIBRARY).getDataSet(name);
    }

    HighFive::DataSet getDynamicsAttributeDataSet(const std::string& name) const {
        if (!dynamicsAttributeNames.count(name)) {
            throw SonataError(fmt::format("No such dynamics attribute: '{}'", name));
        }
        return h5Root.getGroup("0").getGroup(H5_DYNAMICS_PARAMS).getDataSet(name);
    }

    const std::string name;
    const std::string prefix;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
    const std::set<std::string> attributeNames;
    const std::set<std::string> attributeEnumNames;
    const std::set<std::string> dynamicsAttributeNames;
    const IoOpts io_opts;
};

//--------------------------------------------------------------------------------------------------

template <typename Population>
struct PopulationStorage<Population>::Impl {
    Impl(const std::string& _h5FilePath)
        : Impl(_h5FilePath, IoOpts()) {}

    Impl(const std::string& _h5FilePath, const IoOpts& io_opts)
        : Impl(_h5FilePath, std::string(), io_opts) {}

    Impl(const std::string& _h5FilePath, const std::string& _csvFilePath, const IoOpts& io_opts)
        : h5FilePath(_h5FilePath)
        , csvFilePath(_csvFilePath)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup(fmt::format("/{}s", Population::ELEMENT)))
        , io_opts(io_opts) {
        if (!csvFilePath.empty()) {
            throw SonataError(fmt::format("CSV not supported at the moment: {}", csvFilePath));
        }
    }

    const std::string h5FilePath;
    const std::string csvFilePath;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
    const IoOpts io_opts;
};

template <typename Population>
PopulationStorage<Population>::PopulationStorage(const std::string& h5FilePath)
    : PopulationStorage(h5FilePath, IoOpts()) {}

template <typename Population>
PopulationStorage<Population>::PopulationStorage(const std::string& h5FilePath,
                                                 const IoOpts& io_opts)
    : PopulationStorage(h5FilePath, std::string(), io_opts) {}

template <typename Population>
PopulationStorage<Population>::PopulationStorage(const std::string& h5FilePath,
                                                 const std::string& csvFilePath)
    : PopulationStorage(h5FilePath, csvFilePath, IoOpts()) {}

template <typename Population>
PopulationStorage<Population>::PopulationStorage(const std::string& h5FilePath,
                                                 const std::string& csvFilePath,
                                                 const IoOpts& io_opts)
    : impl_([h5FilePath, csvFilePath, io_opts] {
        HDF5_LOCK_GUARD
        return new PopulationStorage::Impl(h5FilePath, csvFilePath, io_opts);
    }()) {}


template <typename Population>
PopulationStorage<Population>::PopulationStorage(PopulationStorage<Population>&& st) noexcept =
    default;


template <typename Population>
PopulationStorage<Population>::~PopulationStorage() noexcept = default;


template <typename Population>
std::set<std::string> PopulationStorage<Population>::populationNames() const {
    HDF5_LOCK_GUARD
    return _listChildren(impl_->h5Root);
}


template <typename Population>
std::shared_ptr<Population> PopulationStorage<Population>::openPopulation(
    const std::string& name) const {
    {
        HDF5_LOCK_GUARD
        if (!impl_->h5Root.exist(name)) {
            throw SonataError(fmt::format("No such population: '{}'", name));
        }
    }
    return std::make_shared<Population>(impl_->h5FilePath,
                                        impl_->csvFilePath,
                                        name,
                                        impl_->io_opts);
}

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
