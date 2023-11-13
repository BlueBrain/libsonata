/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
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

Selection resolve(const HighFive::Group& indexGroup, NodeID nodeID, const Hdf5Reader& reader);
Selection resolve(const HighFive::Group& indexGroup,
                  const std::vector<NodeID>& nodeIDs,
                  const Hdf5Reader& reader);

void write(HighFive::Group& h5Root,
           uint64_t sourceNodeCount,
           uint64_t targetNodeCount,
           bool overwrite);

}  // namespace edge_index
}  // namespace sonata
}  // namespace bbp
