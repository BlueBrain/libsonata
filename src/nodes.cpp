/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include "population.hpp"

#include <fmt/format.h>

#include <bbp/sonata/common.h>
#include <bbp/sonata/nodes.h>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

NodePopulation::NodePopulation(const std::string& h5FilePath,
                               const std::string& csvFilePath,
                               const std::string& name)
    : Population(h5FilePath, csvFilePath, name, ELEMENT) {}


namespace {
template <typename T>
Selection _getMatchingSelection(const std::vector<T>& values, const T value) {
    Selection::Values idx;
    Selection::Value id = 0;

    for (const auto& v : values) {
        if (v == value) {
            idx.push_back(id);
        }
        ++id;
    }
    return Selection::fromValues(idx);
}

template <typename T>
Selection _matchAttributeValues(const NodePopulation& population,
                                const std::string& name,
                                T value) {
    return _getMatchingSelection(population.getAttribute<T>(name, population.selectAll()), value);
}
}  // anonymous namespace

template <typename T>
Selection NodePopulation::matchAttributeValues(const std::string& name, const T value) const {
    auto dtype = impl_->getAttributeDataSet(name).getDataType();
    if (dtype == HighFive::AtomicType<int8_t>() || dtype == HighFive::AtomicType<uint8_t>() ||
        dtype == HighFive::AtomicType<int16_t>() || dtype == HighFive::AtomicType<uint16_t>() ||
        dtype == HighFive::AtomicType<int32_t>() || dtype == HighFive::AtomicType<uint32_t>() ||
        dtype == HighFive::AtomicType<int64_t>() || dtype == HighFive::AtomicType<uint64_t>()) {
        return _matchAttributeValues(*this, name, value);
    } else if (dtype == HighFive::AtomicType<float>() || dtype == HighFive::AtomicType<double>()) {
        throw SonataError("Exact comparison for float/double explicitly not supported");
    } else {
        throw SonataError(
            fmt::format("Unexpected datatype for dataset '{}'", _attributeDataType(name)));
    }
}

template <>
Selection NodePopulation::matchAttributeValues<std::string>(const std::string& name,
                                                            const std::string value) const {
    // enum attribute, need to look up in the @library
    if (enumerationNames().count(name) > 0) {
        const auto enum_values = enumerationValues(name);
        const auto pos = std::find(enum_values.begin(), enum_values.end(), value);
        if (pos == enum_values.end()) {
            return Selection({});
        }
        return _getMatchingSelection<size_t>(getEnumeration<size_t>(name, selectAll()),
                                             pos - enum_values.begin());
    }

    auto dtype = impl_->getAttributeDataSet(name).getDataType();
    if (dtype != HighFive::AtomicType<std::string>()) {
        throw SonataError("H5 dataset must be a string");
    }
    // normal, non-enum, attribute
    return _matchAttributeValues<std::string>(*this, name, value);
}

#define INSTANTIATE_TEMPLATE_METHODS(T)                                                 \
    template Selection NodePopulation::matchAttributeValues<T>(const std::string& name, \
                                                               const T value) const;

/* Note: float/double are PURPOSEFULLY not instantiated */

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

//--------------------------------------------------------------------------------------------------

constexpr const char* NodePopulation::ELEMENT;

template class PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
