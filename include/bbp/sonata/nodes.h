/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License.
 *
 * See top-level LICENSE.txt file for details.
 *************************************************************************/

#pragma once

#include "common.h"
#include "population.h"

#include <string>
#include <vector>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

class SONATA_API NodePopulation : public Population
{
public:
    constexpr static const char* ELEMENT = "node";

    NodePopulation(
        const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name
    );
};

//--------------------------------------------------------------------------------------------------

using NodeStorage = PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
