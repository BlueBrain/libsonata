/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License.
 *
 * See top-level LICENSE.txt file for details.
 *************************************************************************/

#include "edge_index.h"

#include <bbp/sonata/common.h>

#include <cstdint>
#include <set>
#include <vector>


namespace bbp {
namespace sonata {
namespace edge_index {

namespace {

typedef std::vector<std::vector<uint64_t>> RawIndex;

const char* SOURCE_INDEX_GROUP = "indices/source_to_target";
const char* TARGET_INDEX_GROUP = "indices/target_to_source";
const char* NODE_ID_TO_RANGES_DSET = "node_id_to_ranges";
const char* RANGE_TO_EDGE_ID_DSET = "range_to_edge_id";

}  // unnamed namespace


const HighFive::Group sourceIndex(const HighFive::Group& h5Root)
{
    if (!h5Root.exist(SOURCE_INDEX_GROUP)) {
        throw SonataError("No source index group found");
    }
    return h5Root.getGroup(SOURCE_INDEX_GROUP);
}


const HighFive::Group targetIndex(const HighFive::Group& h5Root)
{
    if (!h5Root.exist(TARGET_INDEX_GROUP)) {
        throw SonataError("No target index group found");
    }
    return h5Root.getGroup(TARGET_INDEX_GROUP);
}


Selection resolve(const HighFive::Group& indexGroup, const NodeID nodeID)
{
    if (nodeID >= indexGroup.getDataSet(NODE_ID_TO_RANGES_DSET).getSpace().getDimensions()[0]) {
        // Returning empty set for out-of-range node IDs, to be aligned with SYN2 reader implementation
        // TODO: throw a SonataError instead
        return Selection({});
    }

    RawIndex primaryRange;
    indexGroup
        .getDataSet(NODE_ID_TO_RANGES_DSET)
        .select({ nodeID, 0 }, { 1, 2 })
        .read(primaryRange);

    const uint64_t primaryRangeBegin = primaryRange[0][0];
    const uint64_t primaryRangeEnd = primaryRange[0][1];

    if (primaryRangeBegin >= primaryRangeEnd) {
        return Selection({});
    }

    RawIndex secondaryRange;
    indexGroup
        .getDataSet(RANGE_TO_EDGE_ID_DSET)
        .select({ primaryRangeBegin, 0 }, { primaryRangeEnd - primaryRangeBegin, 2 })
        .read(secondaryRange);

    Selection::Ranges ranges;
    ranges.reserve(secondaryRange.size());

    for (const auto& row: secondaryRange) {
        ranges.emplace_back(row[0], row[1]);
    }

    return Selection(std::move(ranges));
}


Selection resolve(const HighFive::Group& indexGroup, const std::vector<NodeID>& nodeIDs)
{
    if (nodeIDs.size() == 1) {
        return resolve(indexGroup, nodeIDs[0]);
    }
    // TODO optimize: bulk read for primary index
    // TODO optimize: range merging
    std::set<EdgeID> merged;
    for (NodeID nodeID : nodeIDs) {
        const auto ids = resolve(indexGroup, nodeID).flatten();
        merged.insert(ids.begin(), ids.end());
    }
    return Selection::fromValues(merged.begin(), merged.end());
}

}
}
} // namespace bbp::sonata::edge_index