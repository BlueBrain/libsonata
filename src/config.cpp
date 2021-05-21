/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *                         Jonas Karlsson <jonas.karlsson@epfl.ch>
 *                         Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include <bbp/sonata/config.h>

#include <algorithm>  // transform
#include <cassert>
#include <fstream>
#include <memory>
#include <regex>
#include <set>
#include <string>

#include <fmt/format.h>
#include <ghc/filesystem.hpp>
#include <nlohmann/json.hpp>

#include "population.hpp"
#include "utils.h"


namespace {
using bbp::sonata::CircuitConfig;
using bbp::sonata::PopulationProperties;
using bbp::sonata::SonataError;

// to be replaced by std::filesystem once C++17 is used
namespace fs = ghc::filesystem;

class PathResolver
{
  public:
    PathResolver(const std::string& basePath)
        : _basePath(fs::absolute(fs::path(basePath))) {}

    std::string toAbsolute(const std::string& pathStr) const {
        const fs::path path(pathStr);
        const auto absolute = path.is_absolute() ? path : fs::absolute(_basePath / path);
        return absolute.lexically_normal().string();
    }

  private:
    const fs::path _basePath;
};


std::map<std::string, std::string> replaceVariables(std::map<std::string, std::string> variables) {
    constexpr size_t maxIterations = 10;

    bool anyChange = true;
    size_t iteration = 0;

    while (anyChange) {
        anyChange = false;
        auto variablesCopy = variables;

        for (const auto& vI : variables) {
            const auto& vIKey = vI.first;
            const auto& vIValue = vI.second;

            for (auto& vJ : variablesCopy) {
                auto& vJValue = vJ.second;
                const auto startPos = vJValue.find(vIKey);

                if (startPos != std::string::npos) {
                    vJValue.replace(startPos, vIKey.length(), vIValue);
                    anyChange = true;
                }
            }
        }
        variables = variablesCopy;

        if (++iteration == maxIterations) {
            throw SonataError(
                "Reached maximum allowed iterations in variable expansion, "
                "possibly infinite recursion.");
        }
    }

    return variables;
}

nlohmann::json expandVariables(const nlohmann::json& json,
                               const std::map<std::string, std::string>& vars) {
    auto jsonFlat = json.flatten();

    // Expand variables in whole json
    for (auto it = jsonFlat.begin(); it != jsonFlat.end(); ++it) {
        auto& value = it.value();
        if (!value.is_string()) {
            continue;
        }

        auto valueStr = value.get<std::string>();

        for (auto& var : vars) {
            const auto& varName = var.first;
            const auto& varValue = var.second;
            const auto startPos = valueStr.find(varName);

            if (startPos != std::string::npos) {
                valueStr.replace(startPos, varName.length(), varValue);
                value = valueStr;
            }
        }
    }

    return jsonFlat.unflatten();
}

template <typename T>
T getJSONValue(const nlohmann::json& json,
               const std::string& key,
               const std::string& defaultValue = std::string()) {
    auto it = json.find(key);
    if (it != json.end() && !it->is_null()) {
        return it.value().get<T>();
    }

    return defaultValue;
}

std::string getJSONPath(const nlohmann::json& json,
                        const std::string& key,
                        const PathResolver& resolver,
                        const std::string& defaultValue = std::string()) {
    auto value = getJSONValue<std::string>(json, key);
    if (!value.empty())
        return resolver.toAbsolute(value);

    return defaultValue;
}

struct Components {
    std::string morphologiesDir;
    std::map<std::string, std::string> alternateMorphologiesDir;
    std::string biophysicalNeuronModelsDir;
};

Components fillComponents(const nlohmann::json& json, const PathResolver& resolver) {
    const auto components = json.at("components");
    Components result;

    result.morphologiesDir = getJSONPath(components, "morphologies_dir", resolver);

    const auto alternateMorphoDir = components.find("alternate_morphologies");
    if (alternateMorphoDir != components.end()) {
        for (auto it = alternateMorphoDir->begin(); it != alternateMorphoDir->end(); ++it) {
            result.alternateMorphologiesDir[it.key()] = resolver.toAbsolute(it.value());
        }
    }

    result.biophysicalNeuronModelsDir =
        getJSONPath(components, "biophysical_neuron_models_dir", resolver);

    return result;
}

struct SubnetworkFiles {
    std::string elements;
    std::string types;
};

std::vector<SubnetworkFiles> fillSubnetwork(const nlohmann::json& networks,
                                            const std::string& prefix,
                                            const PathResolver& resolver) {
    std::vector<SubnetworkFiles> output;

    const std::string component = prefix + "s";
    const std::string elementsFile = prefix + "s_file";
    const std::string typesFile = prefix + "_types_file";

    const auto iter = networks.find(component);
    for (const auto& node : *iter) {
        auto h5File = getJSONPath(node, elementsFile, resolver);
        if (h5File.empty())
            throw SonataError(
                fmt::format("'{}' network do not define '{}' entry", prefix, elementsFile));

        auto csvFile = getJSONPath(node, typesFile, resolver);

        output.emplace_back(SubnetworkFiles{h5File, csvFile});
    }

    return output;
}

template <typename Population>
std::map<std::string, PopulationProperties> fillPopulationProperties(
    const nlohmann::json& network,
    const PathResolver& resolver,
    const Components& defaultComponents,
    const std::string& defaultPopulationType) {
    std::map<std::string, PopulationProperties> output;

    // Iterate over all defined subnetworks
    for (const auto& node : network) {
        const auto populationsIt = node.find("populations");
        if (populationsIt == node.end())
            continue;

        // Iterate over all defined populations
        for (auto it = populationsIt->begin(); it != populationsIt->end(); ++it) {
            const auto& popData = it.value();
            if (popData.empty())
                continue;

            std::cout << "Overriding " << it.key() << std::endl;
            PopulationProperties& popProperties = output[it.key()];

            // Take population-specific components, if any. Otherwise, fall back to default
            popProperties.type = getJSONValue<std::string>(popData, "type", defaultPopulationType);
            popProperties.morphologiesDir = getJSONPath(popData,
                                                        "morphologies_dir",
                                                        resolver,
                                                        defaultComponents.morphologiesDir);
            popProperties.biophysicalNeuronModelsDir =
                getJSONPath(popData,
                            "biophysical_neuron_models_dir",
                            resolver,
                            defaultComponents.biophysicalNeuronModelsDir);

            // Copy default values
            popProperties.alternateMorphologyFormats = defaultComponents.alternateMorphologiesDir;
            // Overwrite those specified, if any
            const auto altMorphoDir = popData.find("alternate_morphologies");
            if (altMorphoDir != popData.end()) {
                for (auto it = altMorphoDir->begin(); it != altMorphoDir->end(); ++it) {
                    popProperties.alternateMorphologyFormats[it.key()] = resolver.toAbsolute(
                        it.value());
                }
            }
        }
    }

    return output;
}

using Variables = std::map<std::string, std::string>;

Variables readVariables(const nlohmann::json& json) {
    Variables variables;

    if (json.find("networks") == json.end()) {
        return variables;
    }

    const auto manifest = json["manifest"];

    const std::regex regexVariable(R"(\$[a-zA-Z0-9_]*)");

    for (auto it = manifest.begin(); it != manifest.end(); ++it) {
        const auto name = it.key();

        if (std::regex_match(name, regexVariable)) {
            variables[name] = it.value();
        } else {
            throw SonataError(fmt::format("Invalid variable `{}`", name));
        }
    }

    return variables;
}

nlohmann::json parseSonataJson(const std::string& contents) {
    const auto json = nlohmann::json::parse(contents);

    const auto vars = replaceVariables(readVariables(json));
    return expandVariables(json, vars);
}


template <typename Population>
std::map<std::string, SubnetworkFiles> resolvePopulations(
    const std::vector<SubnetworkFiles>& networkNodes) {
    using PopulationStorage = bbp::sonata::PopulationStorage<Population>;
    std::map<std::string, SubnetworkFiles> result;

    for (const auto& network : networkNodes) {
        const auto population = PopulationStorage(network.elements, network.types);
        for (auto name : population.populationNames()) {
            result[name] = network;
        }
    }
    return result;
}

template <typename Population>
std::set<std::string> listPopulations(const std::vector<SubnetworkFiles>& networkNodes) {
    std::set<std::string> result;
    const auto names = resolvePopulations<Population>(networkNodes);
    std::transform(names.begin(),
                   names.end(),
                   std::inserter(result, result.end()),
                   [](std::pair<std::string, SubnetworkFiles> p) { return p.first; });
    return result;
}

template <typename Population>
void checkDuplicatePopulationNames(const std::vector<SubnetworkFiles>& networkNodes) {
    using PopulationStorage = bbp::sonata::PopulationStorage<Population>;
    std::set<std::string> check;

    for (const auto& network : networkNodes) {
        const auto population = PopulationStorage(network.elements, network.types);
        for (auto name : population.populationNames()) {
            if (check.find(name) != check.end())
                throw SonataError(
                    fmt::format("Duplicate population name '{}' in node network file '{}'",
                                name,
                                network.elements));
            check.insert(name);
        }
    }
}

void checkBiophysicalNodePopulations(
    const std::vector<SubnetworkFiles>& networkFiles,
    const std::map<std::string, PopulationProperties>& nodePopulations) {
    const std::string bioType("biophysical");

    // Check that all node populations with type 'biophysical' have a morphologyDir defined
    for (const auto& network : networkFiles) {
        const auto population = bbp::sonata::NodeStorage(network.elements, network.types);
        for (auto name : population.populationNames()) {
            // Check if there is a population that does not override default components,
            // or if there is any population which overrides default components, with
            // biophysical type and that does not define a morphology dir
            auto overridenIt = nodePopulations.find(name);
            if (overridenIt == nodePopulations.end() ||
                (overridenIt != nodePopulations.end() && overridenIt->second.type == bioType &&
                 overridenIt->second.morphologiesDir.empty())) {
                throw SonataError(
                    fmt::format("Node population '{}' is defined as 'biophysical' "
                                "but does not define 'morphology_dir'",
                                name));
            }
        }
    }
}

}  // namespace


namespace bbp {
namespace sonata {

struct CircuitConfig::Impl {
    PathResolver resolver;

    // Circuit config json string whose paths and variables have been expanded to include
    // manifest variables
    std::string expandedJSON;
    // Path to the nodesets file
    std::string nodeSetsFile;
    // Default components value for all networks
    Components components;
    // Nodes network paths
    std::vector<SubnetworkFiles> networkNodes;
    // Node populations that override default components variables
    std::map<std::string, PopulationProperties> nodePopulationProperties;
    // Edges network paths
    std::vector<SubnetworkFiles> networkEdges;
    // Edge populations that override default components variables
    std::map<std::string, PopulationProperties> edgePopulationProperties;

    Impl(const std::string& contents, const std::string& basePath)
        : resolver(basePath) {
        const auto json = parseSonataJson(contents);
        expandedJSON = json.dump();

        // Retrieve node sets file, if any
        if (json.find("node_sets_file") != json.end()) {
            nodeSetsFile = resolver.toAbsolute(json["node_sets_file"]);
        }

        // Fail if no network entry is defined
        if (json.find("networks") == json.end()) {
            throw SonataError("Error parsing config: `networks` not specified");
        }
        auto networks = json.at("networks");

        // Fail if no networks/nodes entry is defined
        if (networks.find("nodes") == networks.end()) {
            throw SonataError("Error parsing networks config: `nodes` not specified");
        }
        // Fail if no networks/edges entry is defined
        if (networks.find("edges") == networks.end()) {
            throw SonataError("Error parsing networks config: `edges``not specified");
        }

        // Fill struct with default components values
        components = fillComponents(json, resolver);

        // Fill node subnetwork paths
        networkNodes = fillSubnetwork(networks, "node", resolver);
        // Check for duplicate populations on node network
        try {
            checkDuplicatePopulationNames<NodePopulation>(networkNodes);
        } catch (const SonataError& e) {
            throw SonataError(
                fmt::format("Node network duplicate populations check: '{}'", e.what()));
        }

        // Fill node population properties
        nodePopulationProperties = fillPopulationProperties<NodePopulation>(networks.at("nodes"),
                                                                            resolver,
                                                                            components,
                                                                            "biophysical");
        // Check if there is any 'biophysical' population without morphology dir
        // (If thre is a default one, no check is needed)
        if (components.morphologiesDir.empty())
            checkBiophysicalNodePopulations(networkNodes, nodePopulationProperties);

        // Fill edge subnetwork paths
        networkEdges = fillSubnetwork(networks, "edge", resolver);
        // Check for duplicate populations on edge network
        try {
            checkDuplicatePopulationNames<EdgePopulation>(networkEdges);
        } catch (const SonataError& e) {
            throw SonataError(
                fmt::format("Edge network duplicate populations check: '{}'", e.what()));
        }
        // Fill edge population properties
        edgePopulationProperties = fillPopulationProperties<EdgePopulation>(networks.at("edges"),
                                                                            resolver,
                                                                            components,
                                                                            "chemical_synapse");
    }
};

CircuitConfig::CircuitConfig(const std::string& contents, const std::string& basePath)
    : impl(new CircuitConfig::Impl(contents, basePath)) {}

CircuitConfig::CircuitConfig(CircuitConfig&&) = default;
CircuitConfig::~CircuitConfig() = default;

CircuitConfig CircuitConfig::fromFile(const std::string& path) {
    return CircuitConfig(readFile(path), fs::path(path).parent_path());
}

std::string CircuitConfig::getNodeSetsPath() const {
    return impl->nodeSetsFile;
}

std::set<std::string> CircuitConfig::listNodePopulations() const {
    return listPopulations<NodePopulation>(impl->networkNodes);
}

template <typename Population>
Population getPopulation(const std::vector<SubnetworkFiles>& network, const std::string name) {
    const auto names = resolvePopulations<Population>(network);
    const auto it = names.find(name);
    if (it == names.end()) {
        throw SonataError(fmt::format("Could not find population '{}'", name));
    }

    return Population(it->second.elements, it->second.types, name);
}

NodePopulation CircuitConfig::getNodePopulation(const std::string& name) const {
    return getPopulation<NodePopulation>(impl->networkNodes, name);
}

std::set<std::string> CircuitConfig::listEdgePopulations() const {
    return listPopulations<EdgePopulation>(impl->networkEdges);
}

EdgePopulation CircuitConfig::getEdgePopulation(const std::string& name) const {
    return getPopulation<EdgePopulation>(impl->networkEdges, name);
}

PopulationProperties CircuitConfig::getNodePopulationProperties(const std::string& name) const {
    auto populations = listNodePopulations();
    if (populations.find(name) == populations.end())
        throw SonataError(fmt::format("Could not find node population '{}'", name));

    auto popPropertiesIt = impl->nodePopulationProperties.find(name);
    if (popPropertiesIt != impl->nodePopulationProperties.end()) {
        return popPropertiesIt->second;
    }

    return {"biophysical",
            impl->components.biophysicalNeuronModelsDir,
            impl->components.morphologiesDir,
            impl->components.alternateMorphologiesDir};
}

PopulationProperties CircuitConfig::getEdgePopulationProperties(const std::string& name) const {
    auto populations = listEdgePopulations();
    if (populations.find(name) == populations.end())
        throw SonataError(fmt::format("Could not find edge population '{}'", name));

    auto popPropertiesIt = impl->edgePopulationProperties.find(name);
    if (popPropertiesIt != impl->edgePopulationProperties.end()) {
        return popPropertiesIt->second;
    }

    return {"chemical_synapse",
            impl->components.biophysicalNeuronModelsDir,
            impl->components.morphologiesDir,
            impl->components.alternateMorphologiesDir};
}

std::string CircuitConfig::getExpandedJSON() const {
    return impl->expandedJSON;
}


}  // namespace sonata
}  // namespace bbp
