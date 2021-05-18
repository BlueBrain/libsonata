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

#include <map>
#include <memory>  // std::unique_ptr
#include <set>
#include <string>
#include <vector>

#include <bbp/sonata/edges.h>
#include <bbp/sonata/nodes.h>

#include "common.h"


namespace bbp {
namespace sonata {

struct SONATA_API PopulationProperties
{
    std::string type;
    std::string biophysicalNeuronModelsDir;
    std::string morphologiesDir;
    std::map<std::string, std::string> alternateMorphologyFormats;
};

/**
 *  Read access to a SONATA circuit config file.
 */
class SONATA_API CircuitConfig
{
  public:

    /**
     * @brief Parses a SONATA JSON Config file and checks it complies with the specification.
     *
     * @param contents string containing the JSON content of the file
     * @param basePath string the path to the directory where the original file is located in
     *                 on disk. It is needed for variable expansion.
     *
     * @throws SonataError on:
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    CircuitConfig(const std::string& contents, const std::string& basePath);

    CircuitConfig(CircuitConfig&&);
    CircuitConfig(const CircuitConfig& other) = delete;
    ~CircuitConfig();

    /**
     * @brief Loads a SONATA JSON config file from disk and returns a CircuitConfig object which
     *        parses it.
     *
     * @throws SonataError on:
     *          - Non accesible file (does not exists / does not have read access)
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    static CircuitConfig fromFile(const std::string& path);

    /**
     * @brief Returns the path to the node sets file.
     *
     * @returns a string with the path to the node sets file, or an empty string if none
     */
    std::string getNodeSetsPath() const;

    /**
     * @brief Returns a set with all available population names across all the node networks.
     *
     * @returns a std::set with population names.
     */
    std::set<std::string> listNodePopulations() const;

    /**
     * @brief Creates and returns a NodePopulation object, initialized from the given population,
     *        and the node network it belongs to.
     *
     * @param name Name of a node population.
     *
     * @returns A NodePopulation object.
     *
     * @throws SonataError if the given population does not exist in any node network.
     */
    NodePopulation getNodePopulation(const std::string& name) const;

    /**
     * @brief Returns a set with all available population names across all the edge networks.
     *
     * @returns a std::set with population names.
     */
    std::set<std::string> listEdgePopulations() const;

    /**
     * @brief Creates and returns an EdgePopulation object, initialized from the given population,
     *        and the edge network it belongs to.
     *
     * @param name Name of an edge population.
     *
     * @returns An EdgePopulation object.
     *
     * @throws SonataError if the given population does not exist in any edge network.
     */
    EdgePopulation getEdgePopulation(const std::string& name) const;

    /**
     * @brief Return a structure containing node population specific properties, falling
     *        back to network properties if there are no population-specific ones.
     *
     * @param name Nam of a node population.
     *
     * @return The properties of the given population.
     *
     * @throws SonataError if the given population name does not correspond to any existing
     *         node population.
     */
    PopulationProperties getNodePopulationProperties(const std::string& name) const;

    /**
     * @brief Return a structure containing edge population specific properties, falling
     *        back to network properties if there are no population-specific ones.
     *
     * @param name Nam of an edge population.
     *
     * @return The properties of the given population.
     *
     * @throws SonataError if the given population name does not correspond to any existing
     *         edge population.
     */
    PopulationProperties getEdgePopulationProperties(const std::string& name) const;

    /**
     * @brief Returns the configuration file JSON whose variables have been expanded by the
     *        manifest entries.
     *
     * @returns a JSON string with the expanded configuration file.
     */
    std::string getExpandedJSON() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}  // namespace sonata
}  // namespace bbp
