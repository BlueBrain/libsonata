#include "population.hpp"
#include "hdf5_mutex.hpp"

#include <bbp/sonata/common.h>
#include <bbp/sonata/edges.h>

#include <fmt/format.h>
#include <highfive/H5File.hpp>

#include <algorithm>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

EdgePopulation::EdgePopulation(
    const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name
)
    : Population(h5FilePath, csvFilePath, name, ELEMENT)
{
}


std::string EdgePopulation::source() const
{
    HDF5_LOCK_GUARD
    std::string result;
    impl_->h5Root.getDataSet("source_node_id").getAttribute("node_population").read(result);
    return result;
}


std::string EdgePopulation::target() const
{
    HDF5_LOCK_GUARD
    std::string result;
    impl_->h5Root.getDataSet("target_node_id").getAttribute("node_population").read(result);
    return result;
}


std::vector<NodeID> EdgePopulation::sourceNodeIDs(const Selection& selection) const
{
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet("source_node_id");
    return _readSelection<NodeID>(dset, selection);
}


std::vector<NodeID> EdgePopulation::targetNodeIDs(const Selection& selection) const
{
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet("target_node_id");
    return _readSelection<NodeID>(dset, selection);
}


namespace {

Selection _resolveIndex(const HighFive::Group& indexGroup, const NodeID nodeID)
{
    typedef std::vector<std::vector<uint64_t>> RawIndex;

    if (nodeID >= indexGroup.getDataSet("node_id_to_ranges").getSpace().getDimensions()[0]) {
        // Returning empty set for out-of-range node IDs, to be aligned with SYN2 reader implementation
        // TODO: throw a SonataError instead
        return Selection({});
    }

    RawIndex primaryRange;
    indexGroup
        .getDataSet("node_id_to_ranges")
        .select({ nodeID, 0 }, { 1, 2 })
        .read(primaryRange);

    const uint64_t primaryRangeBegin = primaryRange[0][0];
    const uint64_t primaryRangeEnd = primaryRange[0][1];

    if (primaryRangeBegin >= primaryRangeEnd) {
        return Selection({});
    }

    RawIndex secondaryRange;
    indexGroup
        .getDataSet("range_to_edge_id")
        .select({ primaryRangeBegin, 0 }, { primaryRangeEnd - primaryRangeBegin, 2 })
        .read(secondaryRange);

    Selection::Ranges ranges;
    ranges.reserve(secondaryRange.size());

    for (const auto& row: secondaryRange) {
        ranges.emplace_back(row[0], row[1]);
    }

    return Selection(std::move(ranges));
}


Selection _resolveIndex(const HighFive::Group& indexGroup, const std::vector<NodeID>& nodeIDs)
{
    if (nodeIDs.size() == 1) {
        return _resolveIndex(indexGroup, nodeIDs[0]);
    }
    // TODO optimize: bulk read for primary index
    // TODO optimize: range merging
    std::set<EdgeID> result;
    for (NodeID nodeID : nodeIDs) {
        const auto ids = _resolveIndex(indexGroup, nodeID).flatten();
        result.insert(ids.begin(), ids.end());
    }
    return _selectionFromValues(result.begin(), result.end());
}

} // unnamed namespace


Selection EdgePopulation::afferentEdges(const std::vector<NodeID>& target) const
{
    HDF5_LOCK_GUARD
    const auto& indexGroup = impl_->h5Root.getGroup("indices/target_to_source");
    return _resolveIndex(indexGroup, target);
}


Selection EdgePopulation::efferentEdges(const std::vector<NodeID>& source) const
{
    HDF5_LOCK_GUARD
    const auto& indexGroup = impl_->h5Root.getGroup("indices/source_to_target");
    return _resolveIndex(indexGroup, source);
}


Selection EdgePopulation::connectingEdges(const std::vector<NodeID>& source, const std::vector<NodeID>& target) const
{
    // TODO optimize: range intersection
    const auto pre = efferentEdges(source).flatten();
    const auto post = afferentEdges(target).flatten();
    assert(std::is_sorted(pre.begin(), pre.end()));  // return result of _resolveIndex is sorted
    assert(std::is_sorted(post.begin(), post.end()));
    Selection::Values result;
    set_intersection(
        pre.begin(), pre.end(),
        post.begin(), post.end(),
        std::back_inserter(result));
    return Selection::fromValues(result);
}

//--------------------------------------------------------------------------------------------------

constexpr const char* EdgePopulation::ELEMENT;

template class PopulationStorage<EdgePopulation>;

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
