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
    std::vector<T> result(selection.flatSize());

    T* dst = result.data();
    for (const Selection::Range& range : selection.ranges()) {
        assert(range.first < range.second);
        auto chunkSize = static_cast<size_t>(range.second - range.first);
        dset.select({static_cast<size_t>(range.first)}, {chunkSize}).read(dst);
        dst += chunkSize;
    }

    return result;
}

}  // unnamed namespace


struct Population::Impl {
    Impl(const std::string& h5FilePath,
         const std::string&,
         const std::string& _name,
         const std::string& _prefix)
        : name(_name)
        , prefix(_prefix)
        , h5File(h5FilePath)
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
                  : std::set<std::string>{}) {
        size_t groupID = 0;
        while (h5Root.exist(std::to_string(groupID))) {
            ++groupID;
        }
        if (groupID != 1) {
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
};

//--------------------------------------------------------------------------------------------------

template <typename Population>
struct PopulationStorage<Population>::Impl {
    Impl(const std::string& _h5FilePath, const std::string& _csvFilePath)
        : h5FilePath(_h5FilePath)
        , csvFilePath(_csvFilePath)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup(fmt::format("/{}s", Population::ELEMENT))) {
        if (!csvFilePath.empty()) {
            throw SonataError("CSV not supported at the moment");
        }
    }

    const std::string h5FilePath;
    const std::string csvFilePath;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
};


template <typename Population>
PopulationStorage<Population>::PopulationStorage(const std::string& h5FilePath,
                                                 const std::string& csvFilePath)
    : impl_([h5FilePath, csvFilePath] {
        HDF5_LOCK_GUARD
        return new PopulationStorage::Impl(h5FilePath, csvFilePath);
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
    return std::make_shared<Population>(impl_->h5FilePath, impl_->csvFilePath, name);
}

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
