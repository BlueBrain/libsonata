/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include "population.hpp"
#include "utils.h"

#include <algorithm>  // std::binary_search, std::max_element, std::any_of
#include <regex>

#include <fmt/format.h>

#include <bbp/sonata/common.h>
#include <bbp/sonata/nodes.h>

namespace bbp {
namespace sonata {

namespace {

template <typename T>
Selection _matchAttributeValues(const NodePopulation& population,
                                const std::string& name,
                                const std::vector<T>& wanted) {
    if (wanted.empty()) {
        return Selection({});
    } else if (wanted.size() == 1) {
        return population.filterAttribute<T>(name,
                                             [&wanted](const T& v) { return wanted[0] == v; });
    } else {
        std::vector<T> wanted_sorted(wanted);
        std::sort(wanted_sorted.begin(), wanted_sorted.end());

        const auto pred = [&wanted_sorted](const T& v) {
            return std::binary_search(wanted_sorted.cbegin(), wanted_sorted.cend(), v);
        };
        return population.filterAttribute<T>(name, pred);
    }
}

bool is_unsigned_int(const HighFive::DataType& dtype) {
    return dtype == HighFive::AtomicType<uint8_t>() || dtype == HighFive::AtomicType<uint16_t>() ||
           dtype == HighFive::AtomicType<uint32_t>() || dtype == HighFive::AtomicType<uint64_t>();
}

bool is_signed_int(const HighFive::DataType& dtype) {
    return dtype == HighFive::AtomicType<int8_t>() || dtype == HighFive::AtomicType<int16_t>() ||
           dtype == HighFive::AtomicType<int32_t>() || dtype == HighFive::AtomicType<int64_t>();
}
bool is_floating(const HighFive::DataType& dtype) {
    return dtype == HighFive::AtomicType<float>() || dtype == HighFive::AtomicType<double>();
}

template <typename UnaryPredicate>
Selection _filterStringAttribute(const NodePopulation& population,
                                 std::string name,
                                 UnaryPredicate pred) {
    if (population.enumerationNames().count(name) > 0) {
        const auto& enum_values = population.enumerationValues(name);
        // it's assumed that the cardinality of a @library is low
        // enough that a std::vector<bool> won't be too large
        std::vector<bool> wanted_enum_mask(enum_values.size());

        bool has_elements = false;
        for (size_t i = 0; i < enum_values.size(); ++i) {
            if (pred(enum_values[i])) {
                wanted_enum_mask[i] = true;
                has_elements = true;
            }
        }

        if (!has_elements) {
            return Selection({});
        }

        const auto& values = population.getEnumeration<size_t>(name, population.selectAll());
        return _getMatchingSelection(values, [&wanted_enum_mask](const size_t v) {
            return wanted_enum_mask.at(v);
        });
    }

    // normal, non-enum, attribute
    return population.filterAttribute<std::string>(name, pred);
}
}  // anonymous namespace

NodePopulation::NodePopulation(const std::string& h5FilePath,
                               const std::string& csvFilePath,
                               const std::string& name)
    : NodePopulation(h5FilePath, csvFilePath, name, Hdf5Reader()) {}

NodePopulation::NodePopulation(const std::string& h5FilePath,
                               const std::string& csvFilePath,
                               const std::string& name,
                               const Hdf5Reader& hdf5_reader)
    : Population(h5FilePath, csvFilePath, name, ELEMENT, hdf5_reader) {}

Selection NodePopulation::regexMatch(const std::string& attribute, const std::string& regex) const {
    std::regex re(regex);
    const auto pred = [re](const std::string& v) {
        std::smatch match;
        std::regex_search(v, match, re);
        return !match.empty();
    };
    return _filterStringAttribute(*this, attribute, pred);
}

template <typename T>
Selection NodePopulation::matchAttributeValues(const std::string& attribute,
                                               const std::vector<T>& values) const {
    if (enumerationNames().count(attribute) > 0) {
        throw SonataError("Matching a @library enum by non-string");
    }

    auto dtype = impl_->getAttributeDataSet(attribute).getDataType();
    if (is_unsigned_int(dtype) || is_signed_int(dtype)) {
        return _matchAttributeValues<T>(*this, attribute, values);
    } else if (is_floating(dtype)) {
        throw SonataError("Exact comparison for float/double explicitly not supported");
    } else {
        throw SonataError(
            fmt::format("Unexpected datatype for dataset '{}'", _attributeDataType(attribute)));
    }
}

template <typename T>
Selection NodePopulation::matchAttributeValues(const std::string& attribute, const T value) const {
    std::vector<T> values{value};
    return matchAttributeValues<T>(attribute, values);
}

template <>
Selection NodePopulation::matchAttributeValues<std::string>(
    const std::string& attribute, const std::vector<std::string>& values) const {
    std::vector<std::string> values_sorted(values);
    std::sort(values_sorted.begin(), values_sorted.end());

    const auto pred = [&values_sorted](const std::string& v) {
        return std::binary_search(values_sorted.cbegin(), values_sorted.cend(), v);
    };

    return _filterStringAttribute(*this, attribute, pred);
}

template <>
Selection NodePopulation::matchAttributeValues<std::string>(const std::string& attribute,
                                                            const std::string value) const {
    std::vector<std::string> values{value};
    return matchAttributeValues<std::string>(attribute, values);
}


#define INSTANTIATE_TEMPLATE_METHODS(T)                                                            \
    template Selection NodePopulation::matchAttributeValues<T>(const std::string&, const T) const; \
    template Selection NodePopulation::matchAttributeValues<T>(const std::string&,                 \
                                                               const std::vector<T>&) const;

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
