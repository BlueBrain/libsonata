/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include "edge_index.h"
#include "hdf5_mutex.hpp"
#include "population.hpp"

#include <bbp/sonata/common.h>
#include <bbp/sonata/edges.h>

#include <fmt/format.h>
#include <highfive/H5File.hpp>

#include <algorithm>


namespace {

const char* const SOURCE_NODE_ID_DSET = "source_node_id";
const char* const TARGET_NODE_ID_DSET = "target_node_id";
const char* const NODE_POPULATION_ATTR = "node_population";

}  // unnamed namespace


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------
//
EdgePopulation::EdgePopulation(const std::string& h5FilePath,
                               const std::string& csvFilePath,
                               const std::string& name)
    : Population(h5FilePath, csvFilePath, name, ELEMENT, Hdf5Reader()) {}

EdgePopulation::EdgePopulation(const std::string& h5FilePath,
                               const std::string& csvFilePath,
                               const std::string& name,
                               const Hdf5Reader& hdf5_reader)
    : Population(h5FilePath, csvFilePath, name, ELEMENT, hdf5_reader) {}


std::string EdgePopulation::source() const {
    HDF5_LOCK_GUARD
    std::string result;
    impl_->h5Root.getDataSet(SOURCE_NODE_ID_DSET).getAttribute(NODE_POPULATION_ATTR).read(result);
    return result;
}


std::string EdgePopulation::target() const {
    HDF5_LOCK_GUARD
    std::string result;
    impl_->h5Root.getDataSet(TARGET_NODE_ID_DSET).getAttribute(NODE_POPULATION_ATTR).read(result);
    return result;
}


std::vector<NodeID> EdgePopulation::sourceNodeIDs(const Selection& selection) const {
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet(SOURCE_NODE_ID_DSET);
    return _readSelection<NodeID>(dset, selection, impl_->hdf5_reader);
}


std::vector<NodeID> EdgePopulation::targetNodeIDs(const Selection& selection) const {
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet(TARGET_NODE_ID_DSET);
    return _readSelection<NodeID>(dset, selection, impl_->hdf5_reader);
}


Selection EdgePopulation::afferentEdges(const std::vector<NodeID>& target) const {
    HDF5_LOCK_GUARD
    return edge_index::resolve(edge_index::targetIndex(impl_->h5Root), target, impl_->hdf5_reader);
}


Selection EdgePopulation::efferentEdges(const std::vector<NodeID>& source) const {
    HDF5_LOCK_GUARD
    return edge_index::resolve(edge_index::sourceIndex(impl_->h5Root), source, impl_->hdf5_reader);
}


Selection EdgePopulation::connectingEdges(const std::vector<NodeID>& source,
                                          const std::vector<NodeID>& target) const {
    // TODO optimize: range intersection
    const auto pre = efferentEdges(source).flatten();
    const auto post = afferentEdges(target).flatten();
    assert(std::is_sorted(pre.begin(), pre.end()));  // return result of _resolveIndex is sorted
    assert(std::is_sorted(post.begin(), post.end()));
    Selection::Values result;
    set_intersection(pre.begin(), pre.end(), post.begin(), post.end(), std::back_inserter(result));
    return Selection::fromValues(result);
}

//--------------------------------------------------------------------------------------------------

void EdgePopulation::writeIndices(const std::string& h5FilePath,
                                  const std::string& population,
                                  uint64_t sourceNodeCount,
                                  uint64_t targetNodeCount,
                                  bool overwrite) {
    HDF5_LOCK_GUARD
    HighFive::File h5File(h5FilePath, HighFive::File::ReadWrite);
    auto h5Root = h5File.getGroup(fmt::format("/edges/{}", population));
    edge_index::write(h5Root, sourceNodeCount, targetNodeCount, overwrite);
}


//--------------------------------------------------------------------------------------------------

constexpr const char* EdgePopulation::ELEMENT;

template class PopulationStorage<EdgePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
