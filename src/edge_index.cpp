/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include "edge_index.h"

#include <bbp/sonata/common.h>

#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>


namespace bbp {
namespace sonata {
namespace edge_index {

namespace {

typedef std::vector<std::vector<uint64_t>> RawIndex;

const char* SOURCE_NODE_ID_DSET = "source_node_id";
const char* TARGET_NODE_ID_DSET = "target_node_id";

const char* INDEX_GROUP = "indices";
const char* SOURCE_INDEX_GROUP = "indices/source_to_target";
const char* TARGET_INDEX_GROUP = "indices/target_to_source";
const char* NODE_ID_TO_RANGES_DSET = "node_id_to_ranges";
const char* RANGE_TO_EDGE_ID_DSET = "range_to_edge_id";

}  // unnamed namespace


const HighFive::Group sourceIndex(const HighFive::Group& h5Root) {
    if (!h5Root.exist(SOURCE_INDEX_GROUP)) {
        throw SonataError("No source index group found");
    }
    return h5Root.getGroup(SOURCE_INDEX_GROUP);
}


const HighFive::Group targetIndex(const HighFive::Group& h5Root) {
    if (!h5Root.exist(TARGET_INDEX_GROUP)) {
        throw SonataError("No target index group found");
    }
    return h5Root.getGroup(TARGET_INDEX_GROUP);
}


Selection resolve(const HighFive::Group& indexGroup, const NodeID nodeID) {
    if (nodeID >= indexGroup.getDataSet(NODE_ID_TO_RANGES_DSET).getSpace().getDimensions()[0]) {
        // Returning empty set for out-of-range node IDs, to be aligned with SYN2 reader
        // implementation
        // TODO: throw a SonataError instead
        return Selection({});
    }

    RawIndex primaryRange;
    indexGroup.getDataSet(NODE_ID_TO_RANGES_DSET)
        .select({static_cast<size_t>(nodeID), 0}, {1, 2})
        .read(primaryRange);

    const uint64_t primaryRangeBegin = primaryRange[0][0];
    const uint64_t primaryRangeEnd = primaryRange[0][1];

    if (primaryRangeBegin >= primaryRangeEnd) {
        return Selection({});
    }

    RawIndex secondaryRange;
    indexGroup.getDataSet(RANGE_TO_EDGE_ID_DSET)
        .select({static_cast<size_t>(primaryRangeBegin), 0},
                {static_cast<size_t>(primaryRangeEnd - primaryRangeBegin), 2})
        .read(secondaryRange);

    Selection::Ranges ranges;
    ranges.reserve(secondaryRange.size());

    for (const auto& row : secondaryRange) {
        ranges.emplace_back(row[0], row[1]);
    }

    return Selection(std::move(ranges));
}


Selection resolve(const HighFive::Group& indexGroup, const std::vector<NodeID>& nodeIDs) {
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


namespace {

std::unordered_map<NodeID, RawIndex> _groupNodeRanges(const std::vector<NodeID>& nodeIDs) {
    std::unordered_map<NodeID, RawIndex> result;

    if (nodeIDs.empty()) {
        return result;
    }

    uint64_t rangeStart = 0;
    NodeID lastNodeID = nodeIDs[rangeStart];
    for (uint64_t i = 1; i < nodeIDs.size(); ++i) {
        if (nodeIDs[i] != lastNodeID) {
            result[lastNodeID].push_back({rangeStart, i});
            rangeStart = i;
            lastNodeID = nodeIDs[rangeStart];
        }
    }

    result[lastNodeID].push_back({rangeStart, nodeIDs.size()});

    return result;
}


std::vector<NodeID> _readNodeIDs(const HighFive::Group& h5Root, const std::string& name) {
    std::vector<NodeID> result;
    h5Root.getDataSet(name).read(result);
    return result;
}


void _writeIndexDataset(const RawIndex& data, const std::string& name, HighFive::Group& h5Group) {
    auto dset = h5Group.createDataSet<uint64_t>(name, HighFive::DataSpace::From(data));
    dset.write(data);
}


void _writeIndexGroup(const std::vector<NodeID>& nodeIDs,
                      uint64_t nodeCount,
                      HighFive::Group& h5Root,
                      const std::string& name) {
    auto indexGroup = h5Root.createGroup(name);

    auto nodeToRanges = _groupNodeRanges(nodeIDs);
    const auto rangeCount =
        std::accumulate(nodeToRanges.begin(),
                        nodeToRanges.end(),
                        uint64_t(0),
                        [](uint64_t total, decltype(nodeToRanges)::const_reference item) {
                            return total + item.second.size();
                        });

    RawIndex primaryIndex;
    RawIndex secondaryIndex;

    primaryIndex.reserve(nodeCount);
    secondaryIndex.reserve(rangeCount);

    uint64_t offset = 0;
    for (NodeID nodeID = 0; nodeID < nodeCount; ++nodeID) {
        const auto it = nodeToRanges.find(nodeID);
        if (it == nodeToRanges.end()) {
            primaryIndex.push_back({offset, offset});
        } else {
            auto& ranges = it->second;
            primaryIndex.push_back({offset, offset + ranges.size()});
            offset += ranges.size();
            std::move(ranges.begin(), ranges.end(), std::back_inserter(secondaryIndex));
        }
    }

    _writeIndexDataset(primaryIndex, NODE_ID_TO_RANGES_DSET, indexGroup);
    _writeIndexDataset(secondaryIndex, RANGE_TO_EDGE_ID_DSET, indexGroup);
}

}  // unnamed namespace


void write(HighFive::Group& h5Root,
           uint64_t sourceNodeCount,
           uint64_t targetNodeCount,
           bool overwrite) {
    if (h5Root.exist(INDEX_GROUP)) {
        if (overwrite) {
            // TODO: remove INDEX_GROUP
            throw SonataError("Index overwrite not implemented yet");
        } else {
            throw SonataError("Index group already exists");
        }
    }

    try {
        _writeIndexGroup(_readNodeIDs(h5Root, SOURCE_NODE_ID_DSET),
                         sourceNodeCount,
                         h5Root,
                         SOURCE_INDEX_GROUP);
        _writeIndexGroup(_readNodeIDs(h5Root, TARGET_NODE_ID_DSET),
                         targetNodeCount,
                         h5Root,
                         TARGET_INDEX_GROUP);
    } catch (...) {
        try {
            // TODO: remove INDEX_GROUP
        } catch (...) {
        }
        throw;
    }
}

}  // namespace edge_index
}  // namespace sonata
}  // namespace bbp
