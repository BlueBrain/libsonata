/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include <algorithm>  // std::copy, std::sort, std::max, std::min
#include <utility>    // std::move

#include "hdf5_mutex.hpp"
#include "utils.h"

#include <fmt/format.h>
#include <highfive/H5File.hpp>

#include "population.hpp"
#include "read_bulk.hpp"

namespace bbp {
namespace sonata {

namespace {

std::string _getDataType(const HighFive::DataSet& dset, const std::string& name) {
    const auto dtype = dset.getDataType();
    if (dtype == HighFive::AtomicType<int8_t>()) {
        return "int8_t";
    } else if (dtype == HighFive::AtomicType<uint8_t>()) {
        return "uint8_t";
    } else if (dtype == HighFive::AtomicType<int16_t>()) {
        return "int16_t";
    } else if (dtype == HighFive::AtomicType<uint16_t>()) {
        return "uint16_t";
    } else if (dtype == HighFive::AtomicType<int32_t>()) {
        return "int32_t";
    } else if (dtype == HighFive::AtomicType<uint32_t>()) {
        return "uint32_t";
    } else if (dtype == HighFive::AtomicType<int64_t>()) {
        return "int64_t";
    } else if (dtype == HighFive::AtomicType<uint64_t>()) {
        return "uint64_t";
    } else if (dtype == HighFive::AtomicType<float>()) {
        return "float";
    } else if (dtype == HighFive::AtomicType<double>()) {
        return "double";
    } else if (dtype == HighFive::AtomicType<std::string>()) {
        return "string";
    } else {
        throw SonataError(fmt::format("Unexpected datatype for dataset '{}'", name));
    }
}

}  // anonymous namespace


Population::Population(const std::string& h5FilePath,
                       const std::string& csvFilePath,
                       const std::string& name,
                       const std::string& prefix,
                       const Hdf5Reader& hdf5_reader)
    : impl_([h5FilePath, csvFilePath, name, prefix, hdf5_reader] {
        HDF5_LOCK_GUARD
        return new Population::Impl(h5FilePath, csvFilePath, name, prefix, hdf5_reader);
    }()) {}


Population::Population(Population&&) noexcept = default;


Population::~Population() noexcept = default;


std::string Population::name() const {
    return impl_->name;
}


uint64_t Population::size() const {
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet(fmt::format("{}_type_id", impl_->prefix));
    return dset.getSpace().getDimensions()[0];
}


Selection Population::selectAll() const {
    return Selection({{0, size()}});
}


const std::set<std::string>& Population::attributeNames() const {
    return impl_->attributeNames;
}


const std::set<std::string>& Population::enumerationNames() const {
    return impl_->attributeEnumNames;
}


std::vector<std::string> Population::enumerationValues(const std::string& name) const {
    HDF5_LOCK_GUARD
    const auto dset = impl_->getLibraryDataSet(name);

    // Note: can't use select all, because our locks aren't re-entrant
    const auto selection = Selection({{0, dset.getSpace().getDimensions()[0]}});
    return _readSelection<std::string>(dset, selection, impl_->hdf5_reader);
}


template <typename T>
std::vector<T> Population::getAttribute(const std::string& name, const Selection& selection) const {
    HDF5_LOCK_GUARD
    return _readSelection<T>(impl_->getAttributeDataSet(name), selection, impl_->hdf5_reader);
}


template <>
std::vector<std::string> Population::getAttribute<std::string>(const std::string& name,
                                                               const Selection& selection) const {
    if (impl_->attributeEnumNames.count(name) == 0) {
        HDF5_LOCK_GUARD
        return _readSelection<std::string>(impl_->getAttributeDataSet(name),
                                           selection,
                                           impl_->hdf5_reader);
    }

    const auto indices = getAttribute<size_t>(name, selection);
    const auto values = enumerationValues(name);

    std::vector<std::string> resolved;
    resolved.reserve(indices.size());

    const auto max = values.size();
    for (const auto& i : indices) {
        if (i >= max) {
            throw SonataError(fmt::format("Invalid enumeration value: {}", i));
        }
        resolved.emplace_back(values[i]);
    }

    return resolved;
}


template <typename T>
std::vector<T> Population::getAttribute(const std::string& name,
                                        const Selection& selection,
                                        const T&) const {
    // with single-group populations default value is not actually used
    return getAttribute<T>(name, selection);
}


template <typename T>
std::vector<T> Population::getEnumeration(const std::string& name,
                                          const Selection& selection) const {
    if (impl_->attributeEnumNames.count(name) == 0) {
        throw SonataError(fmt::format("Invalid enumeration attribute: {}", name));
    }
    if (!std::is_integral<T>::value) {
        throw SonataError(fmt::format("Enumeration attribute '{}' can only be integer", name));
    }

    HDF5_LOCK_GUARD
    return _readSelection<T>(impl_->getAttributeDataSet(name), selection, impl_->hdf5_reader);
}


std::string Population::_attributeDataType(const std::string& name,
                                           bool translate_enumeration) const {
    if (translate_enumeration && impl_->attributeEnumNames.count(name) > 0) {
        return "string";
    }

    HDF5_LOCK_GUARD
    return _getDataType(impl_->getAttributeDataSet(name), name);
}


const std::set<std::string>& Population::dynamicsAttributeNames() const {
    return impl_->dynamicsAttributeNames;
}


template <typename T>
std::vector<T> Population::getDynamicsAttribute(const std::string& name,
                                                const Selection& selection) const {
    HDF5_LOCK_GUARD
    return _readSelection<T>(impl_->getDynamicsAttributeDataSet(name),
                             selection,
                             impl_->hdf5_reader);
}


template <typename T>
std::vector<T> Population::getDynamicsAttribute(const std::string& name,
                                                const Selection& selection,
                                                const T&) const {
    // with single-group populations default value is not actually used
    return getDynamicsAttribute<T>(name, selection);
}


std::string Population::_dynamicsAttributeDataType(const std::string& name) const {
    HDF5_LOCK_GUARD
    return _getDataType(impl_->getDynamicsAttributeDataSet(name), name);
}

template <>
Selection Population::filterAttribute(const std::string& name,
                                      std::function<bool(const std::string)> pred) const {
    auto dtype = impl_->getAttributeDataSet(name).getDataType();
    if (dtype != HighFive::AtomicType<std::string>()) {
        throw SonataError("H5 dataset must be a string");
    }

    const auto& values = getAttribute<std::string>(name, selectAll());
    return _getMatchingSelection(values, std::move(pred));
}

template <typename T>
Selection Population::filterAttribute(const std::string& name,
                                      std::function<bool(const T)> pred) const {
    const auto& values = getAttribute<T>(name, selectAll());
    return _getMatchingSelection(values, pred);
}


//--------------------------------------------------------------------------------------------------

#define INSTANTIATE_TEMPLATE_METHODS(T)                                                         \
    template std::vector<T> Population::getAttribute<T>(const std::string&, const Selection&)   \
        const;                                                                                  \
    template std::vector<T> Population::getAttribute<T>(const std::string&,                     \
                                                        const Selection&,                       \
                                                        const T&) const;                        \
    template std::vector<T> Population::getEnumeration<T>(const std::string&, const Selection&) \
        const;                                                                                  \
    template std::vector<T> Population::getDynamicsAttribute<T>(const std::string&,             \
                                                                const Selection&) const;        \
    template std::vector<T> Population::getDynamicsAttribute<T>(const std::string&,             \
                                                                const Selection&,               \
                                                                const T&) const;                \
    template Selection Population::filterAttribute<T>(const std::string&,                       \
                                                      std::function<bool(const T)> pred) const;


INSTANTIATE_TEMPLATE_METHODS(float)
INSTANTIATE_TEMPLATE_METHODS(double)

INSTANTIATE_TEMPLATE_METHODS(int8_t)
INSTANTIATE_TEMPLATE_METHODS(uint8_t)
INSTANTIATE_TEMPLATE_METHODS(int16_t)
INSTANTIATE_TEMPLATE_METHODS(uint16_t)
INSTANTIATE_TEMPLATE_METHODS(int32_t)
INSTANTIATE_TEMPLATE_METHODS(uint32_t)
INSTANTIATE_TEMPLATE_METHODS(int64_t)
INSTANTIATE_TEMPLATE_METHODS(uint64_t)

#ifdef __APPLE__
INSTANTIATE_TEMPLATE_METHODS(size_t)
#endif

#undef INSTANTIATE_TEMPLATE_METHODS

/* std:: string already has an Population::getAttribute(
      const std::string& name, const Selection& selection) overload, so
 * can't use the macro defined above, expand the rest by hand
 */
template std::vector<std::string> Population::getAttribute<std::string>(const std::string&,
                                                                        const Selection&,
                                                                        const std::string&) const;
template std::vector<std::string> Population::getEnumeration<std::string>(const std::string&,
                                                                          const Selection&) const;
template std::vector<std::string> Population::getDynamicsAttribute<std::string>(
    const std::string&, const Selection&) const;
template std::vector<std::string> Population::getDynamicsAttribute<std::string>(
    const std::string&, const Selection&, const std::string&) const;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
