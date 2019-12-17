/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License.
 *
 * See top-level LICENSE.txt file for details.
 *************************************************************************/

#pragma once

#include <bbp/sonata/population.h>

#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>

namespace bbp {
namespace sonata {
namespace edge_index {

const HighFive::Group sourceIndex(const HighFive::Group& h5Root);
const HighFive::Group targetIndex(const HighFive::Group& h5Root);

Selection resolve(const HighFive::Group& indexGroup, NodeID nodeID);
Selection resolve(const HighFive::Group& indexGroup, const std::vector<NodeID>& nodeIDs);

void write(HighFive::Group& h5Root,
           uint64_t sourceNodeCount,
           uint64_t targetNodeCount,
           bool overwrite);

}  // namespace edge_index
}  // namespace sonata
}  // namespace bbp