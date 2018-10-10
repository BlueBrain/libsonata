#include "population.hpp"

#include <bbp/sonata/common.h>
#include <bbp/sonata/nodes.h>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

NodePopulation::NodePopulation(
    const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name
)
    : Population(std::unique_ptr<Population::Impl>(
        new Population::Impl(h5FilePath, csvFilePath, name, H5_PREFIX)
    ))
{
}

//--------------------------------------------------------------------------------------------------

constexpr const char* NodePopulation::H5_PREFIX;

template class PopulationStorage<NodePopulation>;

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
