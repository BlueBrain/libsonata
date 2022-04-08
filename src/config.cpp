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

#include <bbp/sonata/config.h>

#include <algorithm>  // transform
#include <cassert>
#include <fstream>
#include <memory>
#include <regex>
#include <set>
#include <string>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "../extlib/filesystem.hpp"
#include "population.hpp"
#include "utils.h"


namespace bbp {
namespace sonata {

namespace {
// to be replaced by std::filesystem once C++17 is used
namespace fs = ghc::filesystem;

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

        for (const auto& var : vars) {
            const auto& varName = var.first;
            const auto& varValue = var.second;
            const auto startPos = valueStr.find(varName);

            if (startPos != std::string::npos) {
                valueStr.replace(startPos, varName.length(), varValue);
                value = fs::path(valueStr).lexically_normal();
            }
        }
    }

    return jsonFlat.unflatten();
}

using Variables = std::map<std::string, std::string>;

Variables readVariables(const nlohmann::json& json) {
    Variables variables;

    if (json.find("manifest") == json.end()) {
        return variables;
    }

    const auto manifest = json["manifest"];

    const std::regex regexVariable(R"(\$[a-zA-Z0-9_]*)");

    for (auto it = manifest.begin(); it != manifest.end(); ++it) {
        const auto& name = it.key();

        if (std::regex_match(name, regexVariable)) {
            variables[name] = it.value();
        } else {
            throw SonataError(fmt::format("Invalid variable `{}`", name));
        }
    }

    return variables;
}

std::string toAbsolute(const fs::path& base, const fs::path& path) {
    const auto absolute = path.is_absolute() ? path : fs::absolute(base / path);
    return absolute.lexically_normal().string();
}

}  // namespace

class CircuitConfig::Parser
{
  public:
    using Subnetworks = std::vector<CircuitConfig::SubnetworkFiles>;
    using PopulationOverrides = std::unordered_map<std::string, PopulationProperties>;


    Parser(const std::string& contents, const std::string& basePath)
        : _basePath(fs::absolute(fs::path(basePath))) {
        // Parse and expand JSON string
        const auto rawJson = nlohmann::json::parse(contents);
        const auto vars = replaceVariables(readVariables(rawJson));
        _json = expandVariables(rawJson, vars);
    }

    template <typename T>
    T getJSONValue(const nlohmann::json& json,
                   const std::string& key,
                   const std::string& defaultValue = std::string()) const {
        auto it = json.find(key);
        if (it != json.end() && !it->is_null()) {
            return it.value().get<T>();
        }

        return defaultValue;
    }

    std::string getJSONPath(const nlohmann::json& json,
                            const std::string& key,
                            const std::string& defaultValue = std::string()) const {
        auto value = getJSONValue<std::string>(json, key);
        if (!value.empty()) {
            return toAbsolute(_basePath, value);
        }

        return defaultValue;
    }

    const nlohmann::json& getSubNetworkJson(const std::string& prefix) const {
        // Fail if no network entry is defined
        if (_json.find("networks") == _json.end()) {
            throw SonataError("Error parsing config: `networks` not specified");
        }

        const auto& networks = _json.at("networks");

        const std::string component = prefix + "s";
        if (networks.find(component) == networks.end()) {
            throw SonataError(
                fmt::format("Error parsing networks config: '{}' not specified", component));
        }

        return networks.at(component);
    }

    std::string getNodeSetsPath() const {
        // Retrieve node sets file, if any
        if (_json.find("node_sets_file") != _json.end()) {
            return toAbsolute(_basePath, _json["node_sets_file"]);
        }

        return std::string();
    }

    CircuitConfig::Components parseDefaultComponents() const {
        CircuitConfig::Components result;
        if (_json.find("components") == _json.end()) {
            return result;
        }
        const auto& components = _json.at("components");

        result.morphologiesDir = getJSONPath(components, "morphologies_dir");

        const auto alternateMorphoDir = components.find("alternate_morphologies");
        if (alternateMorphoDir != components.end()) {
            for (auto it = alternateMorphoDir->begin(); it != alternateMorphoDir->end(); ++it) {
                result.alternateMorphologiesDir[it.key()] = toAbsolute(_basePath, it.value());
            }
        }

        result.biophysicalNeuronModelsDir = getJSONPath(components,
                                                        "biophysical_neuron_models_dir");

        return result;
    }

    template <typename PopulationType>
    Subnetworks parseSubNetworks(const std::string& prefix) const {
        using PopulationStorage = bbp::sonata::PopulationStorage<PopulationType>;

        const auto& network = getSubNetworkJson(prefix);

        const std::string elementsFile = prefix + "s_file";
        const std::string typesFile = prefix + "_types_file";

        std::vector<SubnetworkFiles> output;
        for (const auto& node : network) {
            auto h5File = getJSONPath(node, elementsFile);
            if (h5File.empty()) {
                throw SonataError(
                    fmt::format("'{}' network do not define '{}' entry", prefix, elementsFile));
            }

            auto csvFile = getJSONPath(node, typesFile);

            output.emplace_back(SubnetworkFiles{
                h5File, csvFile, PopulationStorage(h5File, csvFile).populationNames()});
        }

        return output;
    }

    Subnetworks parseNodeNetwork() const {
        return parseSubNetworks<NodePopulation>("node");
    }

    Subnetworks parseEdgeNetwork() const {
        return parseSubNetworks<EdgePopulation>("edge");
    }

    template <typename Population>
    PopulationOverrides parsePopulationProperties(const std::string& prefix) const {
        const auto& network = getSubNetworkJson(prefix);

        std::unordered_map<std::string, PopulationProperties> output;

        // Iterate over all defined subnetworks
        for (const auto& node : network) {
            const auto populationsIt = node.find("populations");
            if (populationsIt == node.end()) {
                continue;
            }

            // Iterate over all defined populations
            for (auto it = populationsIt->begin(); it != populationsIt->end(); ++it) {
                const auto& popData = it.value();
                if (popData.empty()) {
                    continue;
                }

                PopulationProperties& popProperties = output[it.key()];

                popProperties.type = getJSONValue<std::string>(popData, "type");
                popProperties.morphologiesDir = getJSONPath(popData, "morphologies_dir");
                popProperties.biophysicalNeuronModelsDir =
                    getJSONPath(popData, "biophysical_neuron_models_dir");

                // Overwrite those specified, if any
                const auto altMorphoDir = popData.find("alternate_morphologies");
                if (altMorphoDir != popData.end()) {
                    for (auto it = altMorphoDir->begin(); it != altMorphoDir->end(); ++it) {
                        popProperties.alternateMorphologyFormats[it.key()] = toAbsolute(_basePath,
                                                                                        it.value());
                    }
                }
            }
        }

        return output;
    }

    PopulationOverrides parseNodePopulations() const {
        return parsePopulationProperties<NodePopulation>("node");
    }

    PopulationOverrides parseEdgePopulations() const {
        return parsePopulationProperties<EdgePopulation>("edge");
    }

    std::string getExpandedJSON() {
        return _json.dump();
    }

  private:
    const fs::path _basePath;
    nlohmann::json _json;
};

class CircuitConfig::PopulationResolver
{
  public:
    static std::set<std::string> listPopulations(const std::vector<SubnetworkFiles>& src) {
        std::set<std::string> result;
        for (const auto& subNetwork : src) {
            result.insert(subNetwork.populations.begin(), subNetwork.populations.end());
        }
        return result;
    }

    template <typename PopulationType>
    static PopulationType getPopulation(const std::string& populationName,
                                        const std::vector<SubnetworkFiles>& src) {
        for (const auto& subNetwork : src) {
            for (const auto& population : subNetwork.populations) {
                if (population == populationName) {
                    return PopulationType(subNetwork.elements, subNetwork.types, populationName);
                }
            }
        }

        throw SonataError(fmt::format("Could not find population '{}'", populationName));
    }

    static void checkDuplicatePopulations(const std::vector<SubnetworkFiles>& src) {
        std::set<std::string> check;
        for (const auto& subNetwork : src) {
            for (const auto& population : subNetwork.populations) {
                if (check.find(population) != check.end()) {
                    throw SonataError(fmt::format("Duplicate population name '{}'", population));
                }
                check.insert(population);
            }
        }
    }

    static void checkBiophysicalPopulations(
        const std::vector<SubnetworkFiles>& src,
        const std::unordered_map<std::string, PopulationProperties>& populations) {
        for (const auto& subNetwork : src) {
            for (const auto& population : subNetwork.populations) {
                const auto it = populations.find(population);
                if (it == populations.end() ||
                    (it != populations.end() && it->second.type == "biophysical" &&
                     it->second.morphologiesDir.empty())) {
                    throw SonataError(
                        fmt::format("Node population '{}' is defined as 'biophysical' "
                                    "but does not define 'morphology_dir'",
                                    population));
                }
            }
        }
    }
};

CircuitConfig::CircuitConfig(const std::string& contents, const std::string& basePath) {
    Parser parser(contents, basePath);

    _expandedJSON = parser.getExpandedJSON();

    _components = parser.parseDefaultComponents();

    _nodeSetsFile = parser.getNodeSetsPath();

    // Load node subnetwork and check for duplicate populations
    _networkNodes = parser.parseNodeNetwork();
    PopulationResolver::checkDuplicatePopulations(_networkNodes);

    // Load node population overrides and check biophysical types
    _nodePopulationProperties = parser.parseNodePopulations();

    // Load edge subnetowrk and check for duplicate populations
    _networkEdges = parser.parseEdgeNetwork();
    PopulationResolver::checkDuplicatePopulations(_networkEdges);

    // Load edge population overrides
    _edgePopulationProperties = parser.parseEdgePopulations();

    const auto updateDefaultProperties =
        [&](std::unordered_map<std::string, PopulationProperties>& map,
            const std::string& defaultType) {
            for (auto& entry : map) {
                if (entry.second.type.empty()) {
                    entry.second.type = defaultType;
                }
                if (entry.second.alternateMorphologyFormats.empty()) {
                    entry.second.alternateMorphologyFormats = _components.alternateMorphologiesDir;
                }
                if (entry.second.biophysicalNeuronModelsDir.empty()) {
                    entry.second.biophysicalNeuronModelsDir =
                        _components.biophysicalNeuronModelsDir;
                }
                if (entry.second.morphologiesDir.empty()) {
                    entry.second.morphologiesDir = _components.morphologiesDir;
                }
            }
        };
    updateDefaultProperties(_nodePopulationProperties, "biophysical");
    updateDefaultProperties(_edgePopulationProperties, "chemical_synapse");

    if (_components.morphologiesDir.empty()) {
        PopulationResolver::checkBiophysicalPopulations(_networkNodes, _nodePopulationProperties);
    }
}

CircuitConfig CircuitConfig::fromFile(const std::string& path) {
    return CircuitConfig(readFile(path), fs::path(path).parent_path());
}

const std::string& CircuitConfig::getNodeSetsPath() const {
    return _nodeSetsFile;
}

std::set<std::string> CircuitConfig::listNodePopulations() const {
    return PopulationResolver::listPopulations(_networkNodes);
}

NodePopulation CircuitConfig::getNodePopulation(const std::string& name) const {
    return PopulationResolver::getPopulation<NodePopulation>(name, _networkNodes);
}

std::set<std::string> CircuitConfig::listEdgePopulations() const {
    return PopulationResolver::listPopulations(_networkEdges);
}

EdgePopulation CircuitConfig::getEdgePopulation(const std::string& name) const {
    return PopulationResolver::getPopulation<EdgePopulation>(name, _networkEdges);
}

PopulationProperties CircuitConfig::getNodePopulationProperties(const std::string& name) const {
    auto populations = listNodePopulations();
    if (populations.find(name) == populations.end()) {
        throw SonataError(fmt::format("Could not find node population '{}'", name));
    }

    auto popPropertiesIt = _nodePopulationProperties.find(name);
    if (popPropertiesIt != _nodePopulationProperties.end()) {
        return popPropertiesIt->second;
    }

    return {"biophysical",
            _components.biophysicalNeuronModelsDir,
            _components.morphologiesDir,
            _components.alternateMorphologiesDir};
}

PopulationProperties CircuitConfig::getEdgePopulationProperties(const std::string& name) const {
    auto populations = listEdgePopulations();
    if (populations.find(name) == populations.end()) {
        throw SonataError(fmt::format("Could not find edge population '{}'", name));
    }

    auto popPropertiesIt = _edgePopulationProperties.find(name);
    if (popPropertiesIt != _edgePopulationProperties.end()) {
        return popPropertiesIt->second;
    }

    return {"chemical_synapse",
            _components.biophysicalNeuronModelsDir,
            _components.morphologiesDir,
            _components.alternateMorphologiesDir};
}

const std::string& CircuitConfig::getExpandedJSON() const {
    return _expandedJSON;
}


class SimulationConfig::Parser
{
  public:
    Parser(const std::string& content, const std::string& basePath)
        : _basePath(fs::absolute(fs::path(basePath)).lexically_normal()) {
        // Parse manifest section and expand JSON string
        const auto rawJson = nlohmann::json::parse(content);
        const auto vars = replaceVariables(readVariables(rawJson));
        _json = expandVariables(rawJson, vars);
    }

    template <typename Iterator, typename Type, typename SectionName>
    void parseMandatory(const Iterator& it,
                        const char* name,
                        const SectionName& sn,
                        Type& buf) const {
        const auto element = it.find(name);
        if (element == it.end())
            throw SonataError(fmt::format("Could not find '{}' in '{}'", name, sn));
        buf = element->template get<Type>();
    }

    template <typename Iterator, typename Type>
    void parseOptional(const Iterator& it, const char* name, Type& buf) const {
        const auto element = it.find(name);
        if (element != it.end())
            buf = element->template get<Type>();
    }

    SimulationConfig::Run parseRun() const {
        const auto runIt = _json.find("run");
        if (runIt == _json.end())
            throw SonataError("Could not find 'run' section");

        SimulationConfig::Run result{};
        parseMandatory(*runIt, "tstop", "run", result.tstop);
        parseMandatory(*runIt, "dt", "run", result.dt);
        return result;
    }

    SimulationConfig::Output parseOutput() const {
        SimulationConfig::Output result{};
        result.outputDir = "output";
        result.spikesFile = "out.h5";

        const auto outputIt = _json.find("output");
        if (outputIt != _json.end()) {
            parseOptional(*outputIt, "output_dir", result.outputDir);
            parseOptional(*outputIt, "spikes_file", result.spikesFile);
        }

        result.outputDir = toAbsolute(_basePath, result.outputDir);

        return result;
    }

    std::unordered_map<std::string, SimulationConfig::Report> parseReports() const {
        std::unordered_map<std::string, SimulationConfig::Report> result;

        const auto reportsIt = _json.find("reports");
        if (reportsIt == _json.end())
            return result;

        for (auto it = reportsIt->begin(); it != reportsIt->end(); ++it) {
            auto& report = result[it.key()];
            auto& valueIt = it.value();
            const auto debugStr = fmt::format("report {}", it.key());
            parseMandatory(valueIt, "cells", debugStr, report.cells);
            parseMandatory(valueIt, "type", debugStr, report.type);
            parseMandatory(valueIt, "dt", debugStr, report.dt);
            parseMandatory(valueIt, "start_time", debugStr, report.startTime);
            parseMandatory(valueIt, "end_time", debugStr, report.endTime);
            parseOptional(valueIt, "file_name", report.fileName);
            if (report.fileName.empty())
                report.fileName = it.key() + "_SONATA.h5";
            else {
                const auto extension = fs::path(report.fileName).extension().string();
                if (extension.empty() || extension != ".h5")
                    report.fileName += ".h5";
            }
            report.fileName = toAbsolute(_basePath, report.fileName);
        }

        return result;
    }

    const std::string parseNetwork() const {
        auto val = _json.find("network") != _json.end() ? _json["network"] : "circuit_config.json";
        return toAbsolute(_basePath, val);
    }

  private:
    const fs::path _basePath;
    nlohmann::json _json;
};

SimulationConfig::SimulationConfig(const std::string& content, const std::string& basePath)
    : _jsonContent(content)
    , _basePath(fs::absolute(basePath).lexically_normal().string()) {
    const Parser parser(content, basePath);
    _run = parser.parseRun();
    _output = parser.parseOutput();
    _reports = parser.parseReports();
    _network = parser.parseNetwork();
}

SimulationConfig SimulationConfig::fromFile(const std::string& path) {
    return SimulationConfig(readFile(path), fs::path(path).parent_path());
}

const std::string& SimulationConfig::getBasePath() const noexcept {
    return _basePath;
}

const std::string& SimulationConfig::getJSON() const noexcept {
    return _jsonContent;
}

const SimulationConfig::Run& SimulationConfig::getRun() const noexcept {
    return _run;
}

const SimulationConfig::Output& SimulationConfig::getOutput() const noexcept {
    return _output;
}

const std::string& SimulationConfig::getNetwork() const noexcept {
    return _network;
}

const SimulationConfig::Report& SimulationConfig::getReport(const std::string& name) const {
    const auto it = _reports.find(name);
    if (it == _reports.end())
        throw SonataError(
            fmt::format("The report '{}' is not present in the simulation config file", name));

    return it->second;
}

}  // namespace sonata
}  // namespace bbp
