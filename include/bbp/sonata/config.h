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
 * Stores population-specific network information.
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
    const std::string& getNodeSetsPath() const;

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
    const std::string& getExpandedJSON() const;

  private:
    struct Components {
        std::string morphologiesDir;
        std::unordered_map<std::string, std::string> alternateMorphologiesDir;
        std::string biophysicalNeuronModelsDir;
    };

    struct SubnetworkFiles {
        std::string elements;
        std::string types;
        std::set<std::string> populations;
    };

    class Parser;
    friend class Parser;
    class PopulationResolver;
    friend class PopulationResolver;

    // Circuit config json string whose paths and variables have been expanded to include
    // manifest variables
    std::string _expandedJSON;

    // Path to the nodesets file
    std::string _nodeSetsFile;

    // Default components value for all networks
    Components _components;

    // Nodes network paths
    std::vector<SubnetworkFiles> _networkNodes;
    // Node populations that override default components variables
    std::unordered_map<std::string, PopulationProperties> _nodePopulationProperties;

    // Edges network paths
    std::vector<SubnetworkFiles> _networkEdges;
    // Edge populations that override default components variables
    std::unordered_map<std::string, PopulationProperties> _edgePopulationProperties;
};

/**
 *  Read access to a SONATA simulation config file.
 */
class SONATA_API SimulationConfig
{
  public:
    /**
     * Parameters defining global simulation settings for spike reports
     */
    struct Run {
        /// Biological simulation end time in milliseconds
        double tstop{};
        /// Integration step duration in milliseconds
        double dt{};
    };
    /**
     * Parameters to override simulator output for spike reports
     */
    struct Output {
        /// Spike report file output directory. Default is "output"
        std::string outputDir;
        /// Spike report file name. Default is "out.h5"
        std::string spikesFile;
    };
    /**
     * List of report parameters collected during the simulation
     */
    struct Report {
        /// Node sets on which to report
        std::string cells;
        /// Report type. Possible values: "compartment", "summation", "synapse"
        std::string type;
        /// Interval between reporting steps in milliseconds
        double dt{};
        /// Time to step reporting in milliseconds
        double startTime{};
        /// Time to stop reporting in milliseconds
        double endTime{};
        /// Report filename. Default is "<report name>_SONATA.h5"
        std::string fileName;
    };

    /**
     * Parses a SONATA JSON simulation configuration file.
     *
     * \throws SonataError on:
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     */
    SimulationConfig(const std::string& content, const std::string& basePath);

    /**
     * Loads a SONATA JSON simulation config file from disk and returns a CircuitConfig object
     * which parses it.
     *
     * \throws SonataError on:
     *          - Non accesible file (does not exists / does not have read access)
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     */
    static SimulationConfig fromFile(const std::string& path);

    /**
     * Returns the base path of the simulation config file
     */
    const std::string& getBasePath() const noexcept;

    /**
     * Returns the JSON content of the simulation config file
     */
    const std::string& getJSON() const noexcept;

    /**
     * Returns the Run section of the simulation configuration.
     */
    const Run& getRun() const noexcept;

    /**
     * Returns the Output section of the simulation configuration.
     */
    const Output& getOutput() const noexcept;

    /**
     * Returns the given report parameters.
     *
     * \throws SonataError if the given report name does not correspond with any existing
     *         report.
     */
    const Report& getReport(const std::string& name) const;

    const std::string& getNetwork() const noexcept;

  private:
    // JSON string
    const std::string _jsonContent;
    // Base path of the simulation config file
    const std::string _basePath;

    // Run section
    Run _run;
    // Output section
    Output _output;
    // List of reports
    std::unordered_map<std::string, Report> _reports;
    // Path of circuit config file for the simulation
    std::string _network;

    class Parser;
    friend class Parser;
};

}  // namespace sonata
}  // namespace bbp
