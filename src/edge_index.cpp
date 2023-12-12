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

#include <array>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>

#include "read_bulk.hpp"

namespace bbp {
namespace sonata {
namespace edge_index {

namespace {

using RawIndex = std::vector<std::array<uint64_t, 2>>;

const char* const SOURCE_NODE_ID_DSET = "source_node_id";
const char* const TARGET_NODE_ID_DSET = "target_node_id";

const char* const INDEX_GROUP = "indices";
const char* const SOURCE_INDEX_GROUP = "indices/source_to_target";
const char* const TARGET_INDEX_GROUP = "indices/target_to_source";
const char* const NODE_ID_TO_RANGES_DSET = "node_id_to_ranges";
const char* const RANGE_TO_EDGE_ID_DSET = "range_to_edge_id";

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

Selection resolve(const HighFive::Group& indexGroup,
                  const std::vector<NodeID>& nodeIDs,
                  const Hdf5Reader& reader) {
    auto node2ranges_dset = indexGroup.getDataSet(NODE_ID_TO_RANGES_DSET);
    auto node_dim = node2ranges_dset.getSpace().getDimensions()[0];
    auto sortedNodeIds = nodeIDs;
    bulk_read::detail::erase_if(sortedNodeIds, [node_dim](auto id) {
        // Filter out `nodeIDs[i] >= dims`; because SYN2 used to return an
        // empty range for an out-of-range `nodeId`s.
        return id >= node_dim;
    });
    std::sort(sortedNodeIds.begin(), sortedNodeIds.end());

    auto nodeSelection = Selection::fromValues(sortedNodeIds);
    auto primaryRange = reader.readSelection<std::array<uint64_t, 2>>(node2ranges_dset,
                                                                      nodeSelection,
                                                                      Selection(RawIndex{{0, 2}}));

    bulk_read::detail::erase_if(primaryRange, [](const auto& range) {
        // Filter out any invalid ranges `start >= end`.
        return range[0] >= range[1];
    });

    primaryRange = bulk_read::sortAndMerge(primaryRange);

    auto secondaryRange = reader.readSelection<std::array<uint64_t, 2>>(
        indexGroup.getDataSet(RANGE_TO_EDGE_ID_DSET), primaryRange, RawIndex{{0, 2}});

    // Sort and eliminate empty ranges.
    secondaryRange = bulk_read::sortAndMerge(secondaryRange);

    return Selection(std::move(secondaryRange));
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


// Use only in the writing code below. General purpose reading should use the
// Hdf5Reader interface.
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
                        uint64_t{0},
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
