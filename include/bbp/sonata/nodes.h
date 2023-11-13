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
#include "population.h"

#include <string>
#include <vector>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

class SONATA_API NodePopulation: public Population
{
  public:
    constexpr static const char* ELEMENT = "node";

    NodePopulation(const std::string& h5FilePath,
                   const std::string& csvFilePath,
                   const std::string& name);

    NodePopulation(const std::string& h5FilePath,
                   const std::string& csvFilePath,
                   const std::string& name,
                   const Hdf5Reader& hdf5_reader);

    /**
     * Return selection of where attribute values match value
     *
     * As per node_set predicates, <tt>value</tt> must be one of type:
     *
     * <li>number  H5T_IEEE_*LE, H5T_STD_*LE</li>
     * <li>string  H5T_C_S1</li>
     * <li>bool    H5T_STD_I8LE</li>
     * <li>null    invalid</li>
     *
     * \throw if the attribute dtype is not comparable
     *
     * Note: This does not match dynamics_params datasets
     */
    template <typename T>
    Selection matchAttributeValues(const std::string& attribute, const T values) const;

    /**
     * Like matchAttributeValues, but for vectors of values to match
     */
    template <typename T>
    Selection matchAttributeValues(const std::string& attribute,
                                   const std::vector<T>& values) const;


    /**
     * For named attribute, return a selection where the passed regular expression matches
     */
    Selection regexMatch(const std::string& attribute, const std::string& re) const;
};

//--------------------------------------------------------------------------------------------------

using NodeStorage = PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
