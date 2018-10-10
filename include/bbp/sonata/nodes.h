#pragma once

#include "common.h"
#include "population.h"

#include <string>
#include <vector>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

class NodePopulation : public Population
{
public:
    constexpr static const char* H5_PREFIX = "node";

    NodePopulation(
        const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name
    );
};

//--------------------------------------------------------------------------------------------------

using NodeStorage = PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
