/*************************************************************************
 * Copyright (C) 2018-2021 Blue Brain Project
 *                         Jonas Karlsson <jonas.karlsson@epfl.ch>
 *                         Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include <memory>  // std::unique_ptr
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <bbp/sonata/edges.h>
#include <bbp/sonata/nodes.h>

#include "common.h"


namespace bbp {
namespace sonata {

/**
 * Stores pouplation-specific network information.
 */
struct SONATA_API PopulationProperties {
    /**
     * Population type
     */
    std::string type;

    /**
     * Path to the template HOC files defining the E-Mode
     */
    std::string biophysicalNeuronModelsDir;

    /**
     * Path to the directory containing the morphologies
     */
    std::string morphologiesDir;

    /**
     * Dictionary for alternate directory paths.
     */
    std::unordered_map<std::string, std::string> alternateMorphologyFormats;
};

/**
 *  Read access to a SONATA circuit config file.
 */
class SONATA_API CircuitConfig
{
  public:
    /**
     * Parses a SONATA JSON config file.
     *
     * \throws SonataError on:
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    CircuitConfig(const std::string& contents, const std::string& basePath);

    CircuitConfig(const CircuitConfig& other) = delete;
    CircuitConfig(CircuitConfig&&);

    ~CircuitConfig();

    /**
     * Loads a SONATA JSON config file from disk and returns a CircuitConfig object which
     * parses it.
     *
     * \throws SonataError on:
     *          - Non accesible file (does not exists / does not have read access)
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    static CircuitConfig fromFile(const std::string& path);

    /**
     * Returns the path to the node sets file.
     */
    std::string getNodeSetsPath() const;

    /**
     *  Returns a set with all available population names across all the node networks.
     */
    std::set<std::string> listNodePopulations() const;

    /**
     * Creates and returns a NodePopulation object, initialized from the given population,
     * and the node network it belongs to.
     *
     * \throws SonataError if the given population does not exist in any node network.
     */
    NodePopulation getNodePopulation(const std::string& name) const;

    /**
     * Returns a set with all available population names across all the edge networks.
     */
    std::set<std::string> listEdgePopulations() const;

    /**
     * Creates and returns an EdgePopulation object, initialized from the given population,
     * and the edge network it belongs to.
     *
     * \throws SonataError if the given population does not exist in any edge network.
     */
    EdgePopulation getEdgePopulation(const std::string& name) const;

    /**
     * Return a structure containing node population specific properties, falling
     * back to network properties if there are no population-specific ones.
     *
     * \throws SonataError if the given population name does not correspond to any existing
     *         node population.
     */
    PopulationProperties getNodePopulationProperties(const std::string& name) const;

    /**
     * Return a structure containing edge population specific properties, falling
     * back to network properties if there are no population-specific ones.
     *
     * \throws SonataError if the given population name does not correspond to any existing
     *         edge population.
     */
    PopulationProperties getEdgePopulationProperties(const std::string& name) const;

    /**
     * Returns the configuration file JSON whose variables have been expanded by the
     * manifest entries.
     */
    std::string getExpandedJSON() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}  // namespace sonata
}  // namespace bbp
