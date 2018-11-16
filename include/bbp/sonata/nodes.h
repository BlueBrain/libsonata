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
