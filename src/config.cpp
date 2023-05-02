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
#include <bbp/sonata/optional.hpp>

#include <bbp/sonata/optional.hpp>
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

// Add a specialization of adl_serializer to the nlohmann namespace for conversion from/to
// nonstd::optional
namespace nlohmann {
template <typename T>
struct adl_serializer<nonstd::optional<T>> {
    static void to_json(json& j, const nonstd::optional<T>& opt) {
        if (opt == nonstd::nullopt) {
            j = nullptr;
        } else {
            j = *opt;
        }
    }

    static void from_json(const json& j, nonstd::optional<T>& opt) {
        if (j.is_null()) {
            opt = nonstd::nullopt;
        } else {
            opt = j.get<T>();
        }
    }
};
}  // namespace nlohmann

namespace bbp {
namespace sonata {

NLOHMANN_JSON_SERIALIZE_ENUM(CircuitConfig::ConfigStatus,
                             {{CircuitConfig::ConfigStatus::invalid, nullptr},
                              {CircuitConfig::ConfigStatus::partial, "partial"},
                              {CircuitConfig::ConfigStatus::complete, "complete"}})

NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Conditions::SpikeLocation,
                             {{SimulationConfig::Conditions::SpikeLocation::invalid, nullptr},
                              {SimulationConfig::Conditions::SpikeLocation::soma, "soma"},
                              {SimulationConfig::Conditions::SpikeLocation::AIS, "AIS"}})

NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Run::IntegrationMethod,
                             {{SimulationConfig::Run::IntegrationMethod::invalid, nullptr},
                              {SimulationConfig::Run::IntegrationMethod::euler, 0},
                              {SimulationConfig::Run::IntegrationMethod::nicholson, 1},
                              {SimulationConfig::Run::IntegrationMethod::nicholson_ion, 2}})

NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Output::SpikesSortOrder,
                             {{SimulationConfig::Output::SpikesSortOrder::invalid, nullptr},
                              {SimulationConfig::Output::SpikesSortOrder::none, "none"},
                              {SimulationConfig::Output::SpikesSortOrder::by_id, "by_id"},
                              {SimulationConfig::Output::SpikesSortOrder::by_time, "by_time"}})

NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Report::Sections,
                             {{SimulationConfig::Report::Sections::invalid, nullptr},
                              {SimulationConfig::Report::Sections::soma, "soma"},
                              {SimulationConfig::Report::Sections::axon, "axon"},
                              {SimulationConfig::Report::Sections::dend, "dend"},
                              {SimulationConfig::Report::Sections::apic, "apic"},
                              {SimulationConfig::Report::Sections::all, "all"}})
NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Report::Type,
                             {{SimulationConfig::Report::Type::invalid, nullptr},
                              {SimulationConfig::Report::Type::compartment, "compartment"},
                              {SimulationConfig::Report::Type::summation, "summation"},
                              {SimulationConfig::Report::Type::synapse, "synapse"}})
NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Report::Scaling,
                             {{SimulationConfig::Report::Scaling::invalid, nullptr},
                              {SimulationConfig::Report::Scaling::none, "none"},
                              {SimulationConfig::Report::Scaling::area, "area"}})
NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::Report::Compartments,
                             {{SimulationConfig::Report::Compartments::invalid, nullptr},
                              {SimulationConfig::Report::Compartments::center, "center"},
                              {SimulationConfig::Report::Compartments::all, "all"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    SimulationConfig::InputBase::Module,
    {{SimulationConfig::InputBase::Module::invalid, nullptr},
     {SimulationConfig::InputBase::Module::linear, "linear"},
     {SimulationConfig::InputBase::Module::relative_linear, "relative_linear"},
     {SimulationConfig::InputBase::Module::pulse, "pulse"},
     {SimulationConfig::InputBase::Module::subthreshold, "subthreshold"},
     {SimulationConfig::InputBase::Module::hyperpolarizing, "hyperpolarizing"},
     {SimulationConfig::InputBase::Module::synapse_replay, "synapse_replay"},
     {SimulationConfig::InputBase::Module::seclamp, "seclamp"},
     {SimulationConfig::InputBase::Module::noise, "noise"},
     {SimulationConfig::InputBase::Module::shot_noise, "shot_noise"},
     {SimulationConfig::InputBase::Module::relative_shot_noise, "relative_shot_noise"},
     {SimulationConfig::InputBase::Module::absolute_shot_noise, "absolute_shot_noise"},
     {SimulationConfig::InputBase::Module::ornstein_uhlenbeck, "ornstein_uhlenbeck"},
     {SimulationConfig::InputBase::Module::relative_ornstein_uhlenbeck,
      "relative_ornstein_uhlenbeck"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    SimulationConfig::InputBase::InputType,
    {{SimulationConfig::InputBase::InputType::invalid, nullptr},
     {SimulationConfig::InputBase::InputType::spikes, "spikes"},
     {SimulationConfig::InputBase::InputType::extracellular_stimulation,
      "extracellular_stimulation"},
     {SimulationConfig::InputBase::InputType::current_clamp, "current_clamp"},
     {SimulationConfig::InputBase::InputType::voltage_clamp, "voltage_clamp"},
     {SimulationConfig::InputBase::InputType::conductance, "conductance"}})

NLOHMANN_JSON_SERIALIZE_ENUM(SimulationConfig::SimulatorType,
                             {{SimulationConfig::SimulatorType::invalid, nullptr},
                              {SimulationConfig::SimulatorType::NEURON, "NEURON"},
                              {SimulationConfig::SimulatorType::CORENEURON, "CORENEURON"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    SimulationConfig::ModificationBase::ModificationType,
    {{SimulationConfig::ModificationBase::ModificationType::invalid, nullptr},
     {SimulationConfig::ModificationBase::ModificationType::TTX, "TTX"},
     {SimulationConfig::ModificationBase::ModificationType::ConfigureAllSections,
      "ConfigureAllSections"}})

namespace {
// to be replaced by std::filesystem once C++17 is used
namespace fs = ghc::filesystem;

void raiseOnBiophysicalPopulationsErrors(
    const std::unordered_map<std::string, NodePopulationProperties>& populations) {
    for (const auto& it : populations) {
        const auto& population = it.first;
        const auto& properties = it.second;
        if (properties.type == "biophysical") {
            if (properties.morphologiesDir.empty() &&
                properties.alternateMorphologyFormats.empty()) {
                throw SonataError(
                    fmt::format("Node population '{}' is defined as 'biophysical' "
                                "but does not define 'morphologies_dir' or "
                                "'alternateMorphologyFormats'",
                                population));
            } else if (properties.biophysicalNeuronModelsDir.empty()) {
                throw SonataError(
                    fmt::format("Node population '{}' is defined as 'biophysical' "
                                "but does not define 'biophysical_neuron_models_dir'",
                                population));
            }
        }
    }
}

template <typename PopulationType>
PopulationType getPopulationProperties(
    const std::string& populationName,
    const std::unordered_map<std::string, PopulationType>& populations) {
    auto it = populations.find(populationName);
    if (it == populations.end()) {
        throw SonataError(fmt::format("Could not find population '{}'", populationName));
    }

    return it->second;
}

template <typename PopulationType, typename PopulationPropertiesT>
PopulationType getPopulation(const std::string& populationName,
                             const std::unordered_map<std::string, PopulationPropertiesT>& src) {
    const auto properties = getPopulationProperties(populationName, src);
    return PopulationType(properties.elementsPath, properties.typesPath, populationName);
}

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

template <typename Type, typename std::enable_if<std::is_enum<Type>::value>::type* = nullptr>
void raiseIfInvalidEnum(const char* name,
                        const Type& buf,
                        const std::string& found_value,
                        std::true_type /* tag */) {
    if (buf == Type::invalid) {
        throw SonataError(fmt::format("Invalid value: '{}' for key '{}'", found_value, name));
    }
}

template <typename Type>
void raiseIfInvalidEnum(const char* /*unused*/,
                        const Type& /*unused*/,
                        const std::string& /*unused*/,
                        std::false_type /* tag */) {}

template <typename Type>
void parseMandatory(const nlohmann::json& it,
                    const char* name,
                    const std::string& section_name,
                    Type& buf) {
    const auto element = it.find(name);
    if (element == it.end()) {
        throw SonataError(fmt::format("Could not find '{}' in '{}'", name, section_name));
    }
    buf = element->get<Type>();

    raiseIfInvalidEnum(name, buf, element->dump(), std::is_enum<Type>());
}

template <typename Type>
void parseOptional(const nlohmann::json& it,
                   const char* name,
                   Type& buf,
                   nonstd::optional<Type> default_value = nonstd::nullopt) {
    const auto element = it.find(name);
    if (element != it.end()) {
        buf = element->get<Type>();
        raiseIfInvalidEnum(name, buf, element->dump(), std::is_enum<Type>());
    } else if (default_value != nonstd::nullopt) {
        buf = default_value.value();
    }
}

void parseVariantType(const nlohmann::json& it, variantValueType& var) {
    switch (it.type()) {
    case nlohmann::json::value_t::boolean:
        var = it.get<bool>();
        break;
    case nlohmann::json::value_t::string:
        var = it.get<std::string>();
        break;
    case nlohmann::json::value_t::number_float:
        var = it.get<double>();
        break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
        var = it.get<int>();
        break;
    default:
        throw SonataError("Value type not supported");
    }
}

SimulationConfig::Input parseInputModule(const nlohmann::json& valueIt,
                                         const SimulationConfig::InputBase::Module module,
                                         const std::string& basePath,
                                         int randomSeed,
                                         const std::string& debugStr) {
    using Module = SimulationConfig::InputBase::Module;

    const auto parseCommon = [&](auto& input) {
        input.module = module;
        parseMandatory(valueIt, "input_type", debugStr, input.inputType);
        parseMandatory(valueIt, "delay", debugStr, input.delay);
        parseMandatory(valueIt, "duration", debugStr, input.duration);
        parseMandatory(valueIt, "node_set", debugStr, input.nodeSet);
    };

    switch (module) {
    case Module::linear: {
        SimulationConfig::InputLinear ret;
        parseCommon(ret);
        parseMandatory(valueIt, "amp_start", debugStr, ret.ampStart);
        parseOptional(valueIt, "amp_end", ret.ampEnd, {ret.ampStart});
        return ret;
    }
    case Module::relative_linear: {
        SimulationConfig::InputRelativeLinear ret;
        parseCommon(ret);
        parseMandatory(valueIt, "percent_start", debugStr, ret.percentStart);
        parseOptional(valueIt, "percent_end", ret.percentEnd, {ret.percentStart});
        return ret;
    }
    case Module::pulse: {
        SimulationConfig::InputPulse ret;
        parseCommon(ret);
        parseMandatory(valueIt, "amp_start", debugStr, ret.ampStart);
        parseMandatory(valueIt, "width", debugStr, ret.width);
        parseMandatory(valueIt, "frequency", debugStr, ret.frequency);
        parseOptional(valueIt, "amp_end", ret.ampEnd, {ret.ampStart});
        return ret;
    }
    case Module::subthreshold: {
        SimulationConfig::InputSubthreshold ret;
        parseCommon(ret);
        parseMandatory(valueIt, "percent_less", debugStr, ret.percentLess);
        return ret;
    }
    case Module::noise: {
        SimulationConfig::InputNoise ret;
        parseCommon(ret);
        const auto mean = valueIt.find("mean");
        const auto mean_percent = valueIt.find("mean_percent");

        if (mean != valueIt.end()) {
            parseOptional(valueIt, "mean", ret.mean);
        }

        if (mean_percent != valueIt.end()) {
            parseOptional(valueIt, "mean_percent", ret.meanPercent);
        }

        if (ret.mean.has_value() && ret.meanPercent.has_value()) {
            throw SonataError("Both `mean` or `mean_percent` have values in " + debugStr);
        } else if (!ret.mean.has_value() && !ret.meanPercent.has_value()) {
            throw SonataError("One of `mean` or `mean_percent` need to have a value in " +
                              debugStr);
        }

        parseOptional(valueIt, "variance", ret.variance);
        return ret;
    }
    case Module::shot_noise: {
        SimulationConfig::InputShotNoise ret;
        parseCommon(ret);
        parseMandatory(valueIt, "rise_time", debugStr, ret.riseTime);
        parseMandatory(valueIt, "decay_time", debugStr, ret.decayTime);
        parseOptional(valueIt, "random_seed", ret.randomSeed, {randomSeed});
        parseOptional(valueIt, "dt", ret.dt, {0.25});
        parseMandatory(valueIt, "rate", debugStr, ret.rate);
        parseMandatory(valueIt, "amp_mean", debugStr, ret.ampMean);
        parseMandatory(valueIt, "amp_var", debugStr, ret.ampVar);
        return ret;
    }
    case Module::relative_shot_noise: {
        SimulationConfig::InputRelativeShotNoise ret;
        parseCommon(ret);

        parseMandatory(valueIt, "rise_time", debugStr, ret.riseTime);
        parseMandatory(valueIt, "decay_time", debugStr, ret.decayTime);
        parseOptional(valueIt, "random_seed", ret.randomSeed, {randomSeed});
        parseOptional(valueIt, "dt", ret.dt, {0.25});
        parseMandatory(valueIt, "amp_cv", debugStr, ret.ampCv);
        parseMandatory(valueIt, "mean_percent", debugStr, ret.meanPercent);
        parseMandatory(valueIt, "sd_percent", debugStr, ret.sdPercent);
        return ret;
    }
    case Module::absolute_shot_noise: {
        SimulationConfig::InputAbsoluteShotNoise ret;
        parseCommon(ret);

        parseMandatory(valueIt, "rise_time", debugStr, ret.riseTime);
        parseMandatory(valueIt, "decay_time", debugStr, ret.decayTime);
        parseOptional(valueIt, "random_seed", ret.randomSeed, {randomSeed});
        parseOptional(valueIt, "dt", ret.dt, {0.25});
        parseMandatory(valueIt, "amp_cv", debugStr, ret.ampCv);
        parseMandatory(valueIt, "mean", debugStr, ret.mean);
        parseMandatory(valueIt, "sigma", debugStr, ret.sigma);
        return ret;
    }
    case Module::hyperpolarizing: {
        SimulationConfig::InputHyperpolarizing ret;
        parseCommon(ret);
        return ret;
    }
    case Module::synapse_replay: {
        SimulationConfig::InputSynapseReplay ret;
        parseCommon(ret);
        parseMandatory(valueIt, "spike_file", debugStr, ret.spikeFile);
        parseOptional(valueIt, "source", ret.source);
        ret.spikeFile = toAbsolute(basePath, ret.spikeFile);
        return ret;
    }
    case Module::seclamp: {
        SimulationConfig::InputSeclamp ret;
        parseCommon(ret);
        parseMandatory(valueIt, "voltage", debugStr, ret.voltage);
        parseOptional(valueIt, "series_resistance", ret.seriesResistance, {0.01});
        return ret;
    }
    case Module::ornstein_uhlenbeck: {
        SimulationConfig::InputOrnsteinUhlenbeck ret;
        parseCommon(ret);
        parseMandatory(valueIt, "tau", debugStr, ret.tau);
        parseOptional(valueIt, "reversal", ret.reversal);
        parseOptional(valueIt, "random_seed", ret.randomSeed, {randomSeed});
        parseOptional(valueIt, "dt", ret.dt, {0.25});

        parseMandatory(valueIt, "mean", debugStr, ret.mean);
        parseMandatory(valueIt, "sigma", debugStr, ret.sigma);
        return ret;
    }
    case Module::relative_ornstein_uhlenbeck: {
        SimulationConfig::InputRelativeOrnsteinUhlenbeck ret;
        parseCommon(ret);
        parseMandatory(valueIt, "tau", debugStr, ret.tau);
        parseOptional(valueIt, "reversal", ret.reversal);
        parseOptional(valueIt, "random_seed", ret.randomSeed, {randomSeed});
        parseOptional(valueIt, "dt", ret.dt, {0.25});

        parseMandatory(valueIt, "mean_percent", debugStr, ret.meanPercent);
        parseMandatory(valueIt, "sd_percent", debugStr, ret.sdPercent);
        return ret;
    }
    default:
        throw SonataError("Unknown module for the input_type in " + debugStr);
    }
}

void parseConditionsMechanisms(
    const nlohmann::json& it,
    std::unordered_map<std::string, std::unordered_map<std::string, variantValueType>>& buf) {
    const auto mechIt = it.find("mechanisms");
    if (mechIt == it.end()) {
        return;
    }
    for (auto& scopeIt : mechIt->items()) {
        std::unordered_map<std::string, variantValueType> map_vars;
        for (auto& varIt : scopeIt.value().items()) {
            variantValueType res_val;
            parseVariantType(varIt.value(), res_val);
            map_vars.insert({varIt.key(), res_val});
        }
        buf.insert({scopeIt.key(), map_vars});
    }
}

void parseConditionsModifications(const nlohmann::json& it,
                                  SimulationConfig::ModificationMap& buf) {
    const auto sectionIt = it.find("modifications");
    if (sectionIt == it.end()) {
        return;
    }
    for (auto& mIt : sectionIt->items()) {
        const auto valueIt = mIt.value();
        const auto debugStr = fmt::format("modification {}", mIt.key());

        SimulationConfig::ModificationBase::ModificationType type;
        parseMandatory(valueIt, "type", debugStr, type);

        switch (type) {
        case SimulationConfig::ModificationBase::ModificationType::TTX: {
            SimulationConfig::ModificationTTX result;
            result.type = type;
            parseMandatory(valueIt, "node_set", debugStr, result.nodeSet);
            buf[mIt.key()] = result;
            break;
        }
        case SimulationConfig::ModificationBase::ModificationType::ConfigureAllSections: {
            SimulationConfig::ModificationConfigureAllSections result;
            result.type = type;
            parseMandatory(valueIt, "node_set", debugStr, result.nodeSet);
            parseMandatory(valueIt, "section_configure", debugStr, result.sectionConfigure);
            buf[mIt.key()] = result;
            break;
        }
        default:
            throw SonataError("Unknown modificationn type in " + debugStr);
        }
    }
}

}  // namespace

class CircuitConfig::Parser
{
  public:
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
                   const T& defaultValue = T()) const {
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

    ConfigStatus getCircuitConfigStatus() const {
        if (_json.find("metadata") == _json.end()) {
            return ConfigStatus::complete;
        }
        const auto& metadata = _json.at("metadata");
        const auto res = getJSONValue<ConfigStatus>(metadata, "status", {ConfigStatus::complete});

        if (res == ConfigStatus::invalid) {
            throw SonataError("Invalid value for `metadata::ConfigStatus` in config");
        }

        return res;
    }

    nlohmann::json getSubNetworkJson(const std::string& prefix,
                                     CircuitConfig::ConfigStatus ConfigStatus) const {
        // Fail if no network entry is defined
        if (_json.find("networks") == _json.end()) {
            if (ConfigStatus == CircuitConfig::ConfigStatus::complete) {
                throw SonataError("Error parsing config: `networks` not specified");
            } else {
                return {};
            }
        }

        const auto& networks = _json.at("networks");

        const std::string component = prefix + "s";
        if (networks.find(component) == networks.end()) {
            if (ConfigStatus == CircuitConfig::ConfigStatus::complete) {
                throw SonataError(
                    fmt::format("Error parsing networks config: '{}' not specified", component));
            } else {
                return {};
            }
        }

        return networks.at(component);
    }

    std::string getNodeSetsPath() const {
        // Retrieve node sets file, if any
        if (_json.find("node_sets_file") != _json.end()) {
            return toAbsolute(_basePath, _json["node_sets_file"]);
        }

        return {};
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

    template <typename PopulationPropertiesT>
    void updateDefaultProperties(std::unordered_map<std::string, PopulationPropertiesT>& map,
                                 const std::string& defaultType,
                                 const CircuitConfig::Components& defaultComponents) {
        for (auto& entry : map) {
            auto& component = entry.second;
            if (component.type.empty()) {
                component.type = defaultType;
            }

            if (component.alternateMorphologyFormats.empty()) {
                component.alternateMorphologyFormats = defaultComponents.alternateMorphologiesDir;
            }

            if (component.biophysicalNeuronModelsDir.empty()) {
                component.biophysicalNeuronModelsDir = defaultComponents.biophysicalNeuronModelsDir;
            }

            if (component.morphologiesDir.empty()) {
                component.morphologiesDir = defaultComponents.morphologiesDir;
            }
        }
    }

    template <typename JSON>
    std::tuple<std::string, std::string> parseSubNetworks(const std::string& prefix,
                                                          const JSON& value) const {
        const std::string elementsFile = prefix + "s_file";
        auto h5File = getJSONPath(value, elementsFile);

        if (h5File.empty()) {
            throw SonataError(
                fmt::format("'{}' network do not define '{}' entry", prefix, elementsFile));
        }

        const std::string typesFile = prefix + "_types_file";
        auto csvFile = getJSONPath(value, typesFile);

        return {h5File, csvFile};
    }

    template <typename PopulationProperties, typename Func>
    std::unordered_map<std::string, PopulationProperties> parsePopulationProperties(
        const std::string& prefix, CircuitConfig::ConfigStatus status, const Func& parser) const {
        const auto& network = getSubNetworkJson(prefix, status);

        std::unordered_map<std::string, PopulationProperties> output;

        for (const auto& subnetwork : network) {
            std::string elementsPath;
            std::string typesPath;
            std::tie(elementsPath, typesPath) = parseSubNetworks(prefix, subnetwork);

            const auto populationsIt = subnetwork.find("populations");
            if (populationsIt == subnetwork.end()) {
                continue;
            }

            for (auto it = populationsIt->begin(); it != populationsIt->end(); ++it) {
                const auto& popData = it.value();

                if (output.find(it.key()) != output.end()) {
                    throw SonataError(fmt::format("Population {} is declared twice", it.key()));
                }

                PopulationProperties& popProperties = output[it.key()];

                popProperties.elementsPath = elementsPath;
                popProperties.typesPath = typesPath;

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

                parser(popProperties, popData);
            }
        }

        return output;
    }

    std::unordered_map<std::string, NodePopulationProperties> parseNodePopulations(
        CircuitConfig::ConfigStatus status) const {
        return parsePopulationProperties<NodePopulationProperties>(
            "node",
            status,
            [&](NodePopulationProperties& popProperties, const nlohmann::json& popData) {
            popProperties.spatialSegmentIndexDir = getJSONPath(popData, "spatial_segment_index_dir");
            });
    }

    std::unordered_map<std::string, EdgePopulationProperties> parseEdgePopulations(
        CircuitConfig::ConfigStatus status) const {
        return parsePopulationProperties<EdgePopulationProperties>(
            "edge",
            status,
            [&](EdgePopulationProperties& popProperties, const nlohmann::json& popData) {
            popProperties.spatialSynapseIndexDir = getJSONPath(popData, "spatial_synapse_index_dir");
            });
    }

    std::string getExpandedJSON() const {
        return _json.dump();
    }

  private:
    const fs::path _basePath;
    nlohmann::json _json;
};

CircuitConfig::CircuitConfig(const std::string& contents, const std::string& basePath) {
    Parser parser(contents, basePath);

    _expandedJSON = parser.getExpandedJSON();
    _status = parser.getCircuitConfigStatus();

    _nodeSetsFile = parser.getNodeSetsPath();

    _nodePopulationProperties = parser.parseNodePopulations(_status);
    _edgePopulationProperties = parser.parseEdgePopulations(_status);

    Components defaultComponents = parser.parseDefaultComponents();
    parser.updateDefaultProperties(_nodePopulationProperties, "biophysical", defaultComponents);
    parser.updateDefaultProperties(_edgePopulationProperties, "chemical", defaultComponents);

    if (getCircuitConfigStatus() == ConfigStatus::complete) {
        raiseOnBiophysicalPopulationsErrors(_nodePopulationProperties);
    }
}

CircuitConfig CircuitConfig::fromFile(const std::string& path) {
    return CircuitConfig(readFile(path), fs::path(path).parent_path());
}

CircuitConfig::ConfigStatus CircuitConfig::getCircuitConfigStatus() const {
    return _status;
}

const std::string& CircuitConfig::getNodeSetsPath() const {
    return _nodeSetsFile;
}

std::set<std::string> CircuitConfig::listNodePopulations() const {
    return getMapKeys(_nodePopulationProperties);
}

NodePopulation CircuitConfig::getNodePopulation(const std::string& name) const {
    return getPopulation<NodePopulation>(name, _nodePopulationProperties);
}

std::set<std::string> CircuitConfig::listEdgePopulations() const {
    return getMapKeys(_edgePopulationProperties);
}

EdgePopulation CircuitConfig::getEdgePopulation(const std::string& name) const {
    return getPopulation<EdgePopulation>(name, _edgePopulationProperties);
}

NodePopulationProperties CircuitConfig::getNodePopulationProperties(const std::string& name) const {
    return getPopulationProperties(name, _nodePopulationProperties);
}

EdgePopulationProperties CircuitConfig::getEdgePopulationProperties(const std::string& name) const {
    return getPopulationProperties(name, _edgePopulationProperties);
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

    SimulationConfig::Run parseRun() const {
        const auto runIt = _json.find("run");
        if (runIt == _json.end()) {
            throw SonataError("Could not find 'run' section");
        }

        SimulationConfig::Run result{};
        parseMandatory(*runIt, "tstop", "run", result.tstop);
        parseMandatory(*runIt, "dt", "run", result.dt);
        parseMandatory(*runIt, "random_seed", "run", result.randomSeed);
        parseOptional(*runIt, "spike_threshold", result.spikeThreshold, {-30});
        parseOptional(*runIt,
                      "integration_method",
                      result.integrationMethod,
                      {Run::IntegrationMethod::euler});
        parseOptional(*runIt, "stimulus_seed", result.stimulusSeed, {0});
        parseOptional(*runIt, "ionchannel_seed", result.ionchannelSeed, {0});
        parseOptional(*runIt, "minis_seed", result.minisSeed, {0});
        parseOptional(*runIt, "synapse_seed", result.synapseSeed, {0});
        return result;
    }

    SimulationConfig::Output parseOutput() const {
        SimulationConfig::Output result{};

        const auto outputIt = _json.find("output");
        if (outputIt == _json.end()) {
            return result;
        }
        parseOptional(*outputIt, "output_dir", result.outputDir, {"output"});
        parseOptional(*outputIt, "log_file", result.logFile, {""});
        parseOptional(*outputIt, "spikes_file", result.spikesFile, {"out.h5"});
        parseOptional(*outputIt,
                      "spikes_sort_order",
                      result.sortOrder,
                      {Output::SpikesSortOrder::by_time});

        result.outputDir = toAbsolute(_basePath, result.outputDir);

        return result;
    }

    SimulationConfig::Conditions parseConditions() const {
        SimulationConfig::Conditions result{};

        const auto conditionsIt = _json.find("conditions");
        if (conditionsIt == _json.end()) {
            return result;
        }
        parseOptional(*conditionsIt, "celsius", result.celsius, {34.0});
        parseOptional(*conditionsIt, "v_init", result.vInit, {-80});
        parseOptional(*conditionsIt,
                      "spike_location",
                      result.spikeLocation,
                      {Conditions::SpikeLocation::soma});
        parseOptional(*conditionsIt, "extracellular_calcium", result.extracellularCalcium);
        parseOptional(*conditionsIt,
                      "randomize_gaba_rise_time",
                      result.randomizeGabaRiseTime,
                      {false});
        parseConditionsMechanisms(*conditionsIt, result.mechanisms);
        parseConditionsModifications(*conditionsIt, result.modifications);
        return result;
    }

    ReportMap parseReports(const SimulationConfig::Output& output) const {
        ReportMap result;

        const auto reportsIt = _json.find("reports");
        if (reportsIt == _json.end()) {
            return result;
        }

        for (auto it = reportsIt->begin(); it != reportsIt->end(); ++it) {
            auto& report = result[it.key()];
            const auto& valueIt = it.value();
            const std::string debugStr = "report " + it.key();

            parseOptional(valueIt, "cells", report.cells, parseNodeSet());
            parseOptional(valueIt, "sections", report.sections, {Report::Sections::soma});
            parseMandatory(valueIt, "type", debugStr, report.type);
            parseOptional(valueIt, "scaling", report.scaling, {Report::Scaling::area});
            parseOptional(valueIt,
                          "compartments",
                          report.compartments,
                          {report.sections == Report::Sections::soma ? Report::Compartments::center
                                                                     : Report::Compartments::all});
            parseMandatory(valueIt, "variable_name", debugStr, report.variableName);
            parseOptional(valueIt, "unit", report.unit, {"mV"});
            parseMandatory(valueIt, "dt", debugStr, report.dt);
            parseMandatory(valueIt, "start_time", debugStr, report.startTime);
            parseMandatory(valueIt, "end_time", debugStr, report.endTime);
            parseOptional(valueIt, "file_name", report.fileName, {it.key() + ".h5"});
            parseOptional(valueIt, "enabled", report.enabled, {true});

            // variable names can look like:
            // `v`, or `i_clamp`, or `Foo.bar` but not `..asdf`, or `asdf..` or `asdf.asdf.asdf`
            const char* const varName = R"(\w+(?:\.?\w+)?)";
            // variable names are separated by `,` with any amount of whitespace separating them
            const std::regex expr(fmt::format(R"({}(?:\s*,\s*{})*)", varName, varName));
            if (!std::regex_match(report.variableName, expr)) {
                throw SonataError(fmt::format("Invalid comma separated variable names '{}'",
                                              report.variableName));
            }

            const auto extension = fs::path(report.fileName).extension().string();
            if (extension.empty() || extension != ".h5") {
                report.fileName += ".h5";
            }
            report.fileName = toAbsolute(output.outputDir, report.fileName);
        }

        return result;
    }

    std::string parseNetwork() const {
        auto val = _json.value("network", "circuit_config.json");
        return toAbsolute(_basePath, val);
    }

    SimulationConfig::SimulatorType parseTargetSimulator() const {
        SimulationConfig::SimulatorType val;
        parseOptional(_json, "target_simulator", val, {SimulationConfig::SimulatorType::NEURON});
        return val;
    }

    std::string parseNodeSetsFile() const noexcept {
        std::string val;
        if (_json.contains("node_sets_file")) {
            val = _json["node_sets_file"];
            return toAbsolute(_basePath, val);
        } else {
            try {
                const auto circuitFile = parseNetwork();
                const auto conf = CircuitConfig::fromFile(circuitFile);
                return conf.getNodeSetsPath();
            } catch (...) {
                // Don't throw CircuitConfig exceptions in SimulationConfig and return empty string
                return val;
            }
        }
    }

    nonstd::optional<std::string> parseNodeSet() const {
        if (_json.contains("node_set")) {
            return {_json["node_set"]};
        } else {
            return nonstd::nullopt;
        }
    }

    InputMap parseInputs() const {
        InputMap result;

        const auto inputsIt = _json.find("inputs");
        if (inputsIt == _json.end()) {
            return result;
        }

        for (auto it = inputsIt->begin(); it != inputsIt->end(); ++it) {
            const auto& valueIt = it.value();
            const auto debugStr = fmt::format("input {}", it.key());

            InputBase::Module module;
            parseMandatory(valueIt, "module", debugStr, module);

            const auto input =
                parseInputModule(valueIt, module, _basePath, parseRun().randomSeed, debugStr);
            result[it.key()] = input;

            auto mismatchingModuleInputType = [&it]() {
                const auto module_name = it->find("module")->get<std::string>();
                const auto input_type = it->find("input_type")->get<std::string>();
                throw SonataError(
                    fmt::format("An `input` has module `{}` and input_type `{}` which mismatch",
                                module_name,
                                input_type));
            };

            auto inputType = nonstd::visit([](const auto& v) { return v.inputType; }, input);
            switch (inputType) {
            case InputBase::InputType::current_clamp: {
                if (!(nonstd::holds_alternative<SimulationConfig::InputLinear>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputRelativeLinear>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputPulse>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputSubthreshold>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputRelativeShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputAbsoluteShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputHyperpolarizing>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputOrnsteinUhlenbeck>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputRelativeOrnsteinUhlenbeck>(
                          input))) {
                    mismatchingModuleInputType();
                }
            } break;
            case InputBase::InputType::spikes:
                if (!nonstd::holds_alternative<SimulationConfig::InputSynapseReplay>(input)) {
                    mismatchingModuleInputType();
                }
                break;
            case InputBase::InputType::voltage_clamp:
                if (!nonstd::holds_alternative<SimulationConfig::InputSeclamp>(input)) {
                    mismatchingModuleInputType();
                }
                break;
            case InputBase::InputType::extracellular_stimulation:
                break;
            case InputBase::InputType::conductance:
                if (!(nonstd::holds_alternative<SimulationConfig::InputShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputRelativeShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputAbsoluteShotNoise>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputOrnsteinUhlenbeck>(input) ||
                      nonstd::holds_alternative<SimulationConfig::InputRelativeOrnsteinUhlenbeck>(
                          input))) {
                    mismatchingModuleInputType();
                }
                break;
            default:
                throw SonataError(fmt::format("Unknown input_type in {}", debugStr));
            }
        }
        return result;
    }

    std::vector<ConnectionOverride> parseConnectionOverrides() const {
        std::vector<ConnectionOverride> result;

        const auto connIt = _json.find("connection_overrides");
        if (connIt == _json.end()) {
            return result;
        }

        if (!connIt->is_array()) {
            throw SonataError("`connection_overrides` must be an array");
        }

        result.reserve(connIt->size());

        for (auto it = connIt->begin(); it != connIt->end(); ++it) {
            const auto& valueIt = it.value();
            ConnectionOverride connect;
            parseMandatory(valueIt, "name", "connection_override", connect.name);
            const auto debugStr = fmt::format("connection_override {}", connect.name);
            parseMandatory(valueIt, "source", debugStr, connect.source);
            parseMandatory(valueIt, "target", debugStr, connect.target);
            parseOptional(valueIt, "weight", connect.weight);
            parseOptional(valueIt, "spont_minis", connect.spontMinis);
            parseOptional(valueIt, "synapse_configure", connect.synapseConfigure);
            parseOptional(valueIt, "modoverride", connect.modoverride);
            parseOptional(valueIt, "synapse_delay_override", connect.synapseDelayOverride);
            parseOptional(valueIt, "delay", connect.delay);
            parseOptional(valueIt, "neuromodulation_dtc", connect.neuromodulationDtc);
            parseOptional(valueIt, "neuromodulation_strength", connect.neuromodulationStrength);

            result.push_back(std::move(connect));
        }
        return result;
    }

    std::unordered_map<std::string, std::string> parseMetaData() const {
        std::unordered_map<std::string, std::string> result;
        const auto metaIt = _json.find("metadata");
        if (metaIt == _json.end()) {
            return result;
        }
        for (auto& it : metaIt->items()) {
            result.insert({it.key(), it.value()});
        }
        return result;
    }

    std::unordered_map<std::string, variantValueType> parseBetaFeatures() const {
        std::unordered_map<std::string, variantValueType> result;
        const auto fIt = _json.find("beta_features");
        if (fIt == _json.end()) {
            return result;
        }
        for (auto& it : fIt->items()) {
            variantValueType res_val;
            parseVariantType(it.value(), res_val);
            result.insert({it.key(), res_val});
        }
        return result;
    }

    std::string getExpandedJSON() const {
        return _json.dump();
    }

  private:
    const fs::path _basePath;
    nlohmann::json _json;
};

SimulationConfig::SimulationConfig(const std::string& content, const std::string& basePath)
    : _basePath(fs::absolute(basePath).lexically_normal().string()) {
    const Parser parser(content, basePath);
    _expandedJSON = parser.getExpandedJSON();
    _run = parser.parseRun();
    _output = parser.parseOutput();
    _conditions = parser.parseConditions();
    _reports = parser.parseReports(_output);
    _network = parser.parseNetwork();
    _inputs = parser.parseInputs();
    _connection_overrides = parser.parseConnectionOverrides();
    _targetSimulator = parser.parseTargetSimulator();
    _nodeSetsFile = parser.parseNodeSetsFile();
    _nodeSet = parser.parseNodeSet();
    _metaData = parser.parseMetaData();
    _betaFeatures = parser.parseBetaFeatures();
}

SimulationConfig SimulationConfig::fromFile(const std::string& path) {
    return SimulationConfig(readFile(path), fs::path(path).parent_path());
}

const std::string& SimulationConfig::getBasePath() const noexcept {
    return _basePath;
}

const SimulationConfig::Run& SimulationConfig::getRun() const noexcept {
    return _run;
}

const SimulationConfig::Output& SimulationConfig::getOutput() const noexcept {
    return _output;
}

const SimulationConfig::Conditions& SimulationConfig::getConditions() const noexcept {
    return _conditions;
}

const std::string& SimulationConfig::getNetwork() const noexcept {
    return _network;
}

std::set<std::string> SimulationConfig::listReportNames() const {
    return getMapKeys(_reports);
}

const SimulationConfig::Report& SimulationConfig::getReport(const std::string& name) const {
    const auto it = _reports.find(name);
    if (it == _reports.end()) {
        throw SonataError(
            fmt::format("The report '{}' is not present in the simulation config file", name));
    }

    return it->second;
}

std::set<std::string> SimulationConfig::listInputNames() const {
    return getMapKeys(_inputs);
}

const SimulationConfig::Input& SimulationConfig::getInput(const std::string& name) const {
    const auto it = _inputs.find(name);
    if (it == _inputs.end()) {
        throw SonataError(
            fmt::format("The input '{}' is not present in the simulation config file", name));
    }
    return it->second;
}

const std::vector<SimulationConfig::ConnectionOverride>& SimulationConfig::getConnectionOverrides()
    const noexcept {
    return _connection_overrides;
}

const SimulationConfig::SimulatorType& SimulationConfig::getTargetSimulator() const {
    return _targetSimulator;
}

const std::string& SimulationConfig::getNodeSetsFile() const noexcept {
    return _nodeSetsFile;
}

const nonstd::optional<std::string>& SimulationConfig::getNodeSet() const noexcept {
    return _nodeSet;
}

const std::unordered_map<std::string, std::string>& SimulationConfig::getMetaData() const noexcept {
    return _metaData;
}

const std::unordered_map<std::string, variantValueType>& SimulationConfig::getBetaFeatures() const
    noexcept {
    return _betaFeatures;
}

const std::string& SimulationConfig::getExpandedJSON() const {
    return _expandedJSON;
}

std::set<std::string> SimulationConfig::Conditions::listModificationNames() const {
    return getMapKeys(modifications);
}

const SimulationConfig::Modification& SimulationConfig::Conditions::getModification(
    const std::string& name) const {
    const auto it = modifications.find(name);
    if (it == modifications.end()) {
        throw SonataError(
            fmt::format("The modification '{}' is not present in the simulation config file",
                        name));
    }
    return it->second;
}

}  // namespace sonata
}  // namespace bbp
