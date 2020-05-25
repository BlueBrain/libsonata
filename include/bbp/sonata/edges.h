/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include "common.h"
#include "population.h"

#include <string>
#include <vector>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

class SONATA_API EdgePopulation: public Population
{
  public:
    constexpr static const char* ELEMENT = "edge";

    EdgePopulation(const std::string& h5FilePath,
                   const std::string& csvFilePath,
                   const std::string& name);

    /**
     * Name of source population extracted from 'source_node_id' dataset
     */
    std::string source() const;

    /**
     * Name of target population extracted from 'source_node_id' dataset
     */
    std::string target() const;

    std::vector<NodeID> sourceNodeIDs(const Selection& selection) const;

    std::vector<NodeID> targetNodeIDs(const Selection& selection) const;

    /**
     * Find inbound edges for a given node ID.
     */
    Selection afferentEdges(const std::vector<NodeID>& target) const;

    /**
     * Find outbound edges for a given node ID.
     */
    Selection efferentEdges(const std::vector<NodeID>& source) const;

    /**
     * Find edges connecting two given nodes.
     */
    Selection connectingEdges(const std::vector<NodeID>& source,
                              const std::vector<NodeID>& target) const;

    /**
     * Write bidirectional node->edge indices to EdgePopulation HDF5.
     */
    static void writeIndices(const std::string& h5FilePath,
                             const std::string& population,
                             uint64_t sourceNodeCount,
                             uint64_t targetNodeCount,
                             bool overwrite = false);
};

//--------------------------------------------------------------------------------------------------

using EdgeStorage = PopulationStorage<EdgePopulation>;

//--------------------------------------------------------------------------------------------------

}  // namespace sonata
}  // namespace bbp
