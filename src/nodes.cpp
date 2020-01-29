/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License.
 *
 * See top-level LICENSE.txt file for details.
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


Selection _matchAttributeValues(const NodePopulation& population,
                                const std::string& name,
                                std::string value) {
    // enum attribute, need to look up in the @library
    if (population.enumerationNames().count(name) > 0) {
        const auto enum_values = population.enumerationValues(name);
        const auto pos = std::find(enum_values.begin(), enum_values.end(), value);
        if (pos == enum_values.end()) {
            return Selection({});
        }
        return _getMatchingSelection<size_t>(
            population.getEnumeration<size_t>(name, population.selectAll()),
            pos - enum_values.begin());
    }

    // normal attribute
    return _matchAttributeValues<std::string>(population, name, value);
}

}  // anonymous namespace

template <typename T>
Selection NodePopulation::matchAttributeValues(const std::string& name, const T value) const {
    // TODO: need to check if enum
    auto dtype = impl_->getAttributeDataSet(name).getDataType();
    if (dtype == HighFive::AtomicType<int8_t>() || dtype == HighFive::AtomicType<uint8_t>() ||
        dtype == HighFive::AtomicType<int16_t>() || dtype == HighFive::AtomicType<uint16_t>() ||
        dtype == HighFive::AtomicType<int32_t>() || dtype == HighFive::AtomicType<uint32_t>() ||
        dtype == HighFive::AtomicType<int64_t>() || dtype == HighFive::AtomicType<uint64_t>() ||
        dtype == HighFive::AtomicType<float>() || dtype == HighFive::AtomicType<double>()) {
        return _matchAttributeValues(*this, name, value);
    } else {
        throw SonataError(
            fmt::format("Unexpected datatype for dataset '{}'", _attributeDataType(name)));
    }
}

template <>
Selection NodePopulation::matchAttributeValues<std::string>(const std::string& name,
                                                            const std::string value) const {
    return _matchAttributeValues(*this, name, value);
}

#define INSTANTIATE_TEMPLATE_METHODS(T)                                                 \
    template Selection NodePopulation::matchAttributeValues<T>(const std::string& name, \
                                                               const T value) const;

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

#undef INSTANTIATE_TEMPLATE_METHODS

//--------------------------------------------------------------------------------------------------

constexpr const char* NodePopulation::ELEMENT;

template class PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
