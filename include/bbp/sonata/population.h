/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include "common.h"

#include <cstdint>
#include <functional>
#include <memory>  // std::shared_ptr, std::unique_ptr
#include <set>
#include <string>
#include <utility>  // std::move
#include <vector>

#include <bbp/sonata/hdf5_reader.h>
#include <bbp/sonata/selection.h>

namespace bbp {
namespace sonata {

class SONATA_API Population
{
  public:
    /**
     * Name of the population used for identifying it in circuit composition
     */
    std::string name() const;

    /**
     * Total number of elements
     */
    uint64_t size() const;

    /**
     * Selection covering all elements
     */
    Selection selectAll() const;

    /**
     * All attribute names (CSV columns + required attributes + union of attributes in groups)
     */
    const std::set<std::string>& attributeNames() const;

    /**
     * All attribute names that are explicit enumerations
     *
     * See also:
     * https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes
     */
    const std::set<std::string>& enumerationNames() const;

    /**
     * Get attribute values for given {element} Selection
     *
     * If string values are requested and the attribute is a explicit
     * enumeration, values will be resolved to strings.
     *
     * See also:
     * https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes
     *
     * \param name is a string to allow attributes not defined in spec
     * \param selection is a selection to retrieve the attribute values from
     * \throw if there is no such attribute for the population
     * \throw if the attribute is not defined for _any_ element from the selection
     */
    template <typename T>
    std::vector<T> getAttribute(const std::string& name, const Selection& selection) const;

    /**
     * Get attribute values for given {element} Selection
     *
     * If string values are requested and the attribute is a explicit
     * enumeration, values will be resolved to strings.
     *
     * See also:
     * https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes
     *
     * \param name is a string to allow attributes not defined in spec
     * \param selection is a selection to retrieve the attribute values from
     * \param default is a value to use for {element}s without the given attribute
     * \throw if there is no such attribute for the population
     */
    template <typename T>
    std::vector<T> getAttribute(const std::string& name,
                                const Selection& selection,
                                const T& defaultValue) const;

    /**
     * Get enumeration values for given attribute and {element} Selection
     *
     * See also:
     * https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes
     *
     * \param name is a string to allow enumeration attributes not defined in spec
     * \param selection is a selection to retrieve the enumeration values from
     * \throw if there is no such attribute for the population
     * \throw if the attribute is not defined for _any_ element from the selection
     */
    template <typename T>
    std::vector<T> getEnumeration(const std::string& name, const Selection& selection) const;

    /**
     * Get all allowed attribute enumeration values
     *
     * \param name is a string to allow attributes not defined in spec
     *
     * \throw if there is no such attribute for the population
     */
    std::vector<std::string> enumerationValues(const std::string& name) const;

    /**
     * Get attribute data type, optionally translating enumeration types

     * \internal
     * It is a helper method for dynamic languages bindings;
     * and is not intended for use in the ordinary client C++ code.
     */
    std::string _attributeDataType(const std::string& name,
                                   bool translate_enumeration = false) const;

    /**
     * All dynamics attribute names (JSON keys + union of attributes in groups)
     */
    const std::set<std::string>& dynamicsAttributeNames() const;

    /**
     * Get dynamics attribute values for given {element} Selection
     *
     * \param name is a string to allow attributes not defined in spec
     * \param selection is a selection to retrieve the dynamics attribute values from
     * \throw if there is no such attribute for the population
     * \throw if the attribute is not defined for _any_ edge from the edge selection
     */
    template <typename T>
    std::vector<T> getDynamicsAttribute(const std::string& name, const Selection& selection) const;

    /**
     * Get dynamics attribute values for given {element} Selection
     *
     * \param name is a string to allow attributes not defined in spec
     * \param selection is a selection to retrieve the dynamics attribute values from
     * \param default is a value to use for {element}s without the given attribute
     * \throw if there is no such attribute for the population
     */
    template <typename T>
    std::vector<T> getDynamicsAttribute(const std::string& name,
                                        const Selection& selection,
                                        const T& defaultValue) const;

    /**
     * Get dynamics attribute data type

     * \internal
     * It is a helper method for dynamic languages bindings;
     * and is not intended for use in the ordinary client C++ code.
     */
    std::string _dynamicsAttributeDataType(const std::string& name) const;

    template <typename T>
    Selection filterAttribute(const std::string& name, std::function<bool(const T)> pred) const;

  protected:
    Population(const std::string& h5FilePath,
               const std::string& csvFilePath,
               const std::string& name,
               const std::string& prefix,
               const Hdf5Reader& hdf5_reader);

    Population(const Population&) = delete;

    Population(Population&&) noexcept;

    virtual ~Population() noexcept;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

template <>
std::vector<std::string> Population::getAttribute<std::string>(const std::string& name,
                                                               const Selection& selection) const;

//--------------------------------------------------------------------------------------------------

/**
 * Collection of {PopulationClass}s stored in a H5 file and optional CSV.
 */
template <typename Population>
class SONATA_API PopulationStorage
{
  public:
    PopulationStorage(const std::string& h5FilePath);
    PopulationStorage(const std::string& h5FilePath, const std::string& csvFilePath);
    PopulationStorage(const std::string& h5FilePath, const Hdf5Reader& hdf5_reader);
    PopulationStorage(const std::string& h5FilePath,
                      const std::string& csvFilePath,
                      const Hdf5Reader& hdf5_reader);

    PopulationStorage(const PopulationStorage&) = delete;

    PopulationStorage(PopulationStorage&&) noexcept;

    ~PopulationStorage() noexcept;

    /**
     * Set of all {PopulationClass} names
     */
    std::set<std::string> populationNames() const;

    /**
     * Open a specific {PopulationClass} by name
     * \param name the name of the population to open
     * \throw if no population with such a name exists
     */
    std::shared_ptr<Population> openPopulation(const std::string& name) const;

  protected:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
