#include "population.hpp"

#include <bbp/sonata/common.h>
#include <bbp/sonata/nodes.h>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

NodePopulation::NodePopulation(
    const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name
)
    : Population(h5FilePath, csvFilePath, name, ELEMENT)
{
}

//--------------------------------------------------------------------------------------------------

constexpr const char* NodePopulation::ELEMENT;

template class PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
