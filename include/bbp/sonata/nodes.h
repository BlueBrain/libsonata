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

    /**
     * Return selection of where attribute values match value
     *
     * As per node_set predicates, value must be one of type:
     *
     *  number	H5T_IEEE_*LE, H5T_STD_*LE
     *  string	H5T_C_S1
     *  bool	H5T_STD_I8LE
     *  null	invalid
     *
     * @throw if the attribute dtype is not comparable
     *
     * Note: This does not match Dynamics_params datasets
     */
    template <typename T>
    Selection matchAttributeValues(const std::string& attribute, const T value) const;
};

//--------------------------------------------------------------------------------------------------

using NodeStorage = PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
