#include <catch2/catch.hpp>

#include <nlohmann/json.hpp>

#include "../extlib/filesystem.hpp"

#include <bbp/sonata/config.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace bbp::sonata;

bool endswith(const std::string& haystack, const std::string& needle) {
    return std::equal(needle.rbegin(), needle.rend(), haystack.rbegin());
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

TEST_CASE("CircuitConfig") {
    SECTION("Simple") {
        const auto config = CircuitConfig::fromFile("./data/config/circuit_config.json");
        CHECK(config.getNodeSetsPath()[0] == '/');  // is an absolute path
        CHECK(endswith(config.getNodeSetsPath(), "node_sets.json"));

        CHECK(config.listNodePopulations() == std::set<std::string>{"nodes-A", "nodes-B"});
        CHECK(config.listEdgePopulations() == std::set<std::string>{"edges-AB", "edges-AC"});

        CHECK_THROWS_AS(config.getNodePopulation("DoesNotExist"), SonataError);
        CHECK_THROWS_AS(config.getEdgePopulation("DoesNotExist"), SonataError);

        CHECK(config.getNodePopulation("nodes-A").name() == "nodes-A");
        CHECK(config.getEdgePopulation("edges-AB").name() == "edges-AB");

        CHECK_THROWS_AS(config.getNodePopulationProperties("DoesNotExist"), SonataError);
        CHECK_THROWS_AS(config.getEdgePopulationProperties("DoesNotExist"), SonataError);

        CHECK(config.getNodePopulationProperties("nodes-A").type == "biophysical");
        CHECK(endswith(config.getNodePopulationProperties("nodes-A").typesPath, ""));
        CHECK(endswith(config.getNodePopulationProperties("nodes-A").elementsPath, "tests/data/nodes1.h5"));

        CHECK(config.getEdgePopulationProperties("edges-AB").type == "chemical");
        CHECK(endswith(config.getEdgePopulationProperties("edges-AB").typesPath, ""));
        CHECK(endswith(config.getEdgePopulationProperties("edges-AB").elementsPath, "tests/data/edges1.h5"));

        CHECK_NOTHROW(nlohmann::json::parse(config.getExpandedJSON()));
        CHECK(nlohmann::json::parse(config.getExpandedJSON())
                  .at("components")
                  .at("morphologies_dir")
                  .get<std::string>() == "morphologies");
    }

    SECTION("Exception") {
        CHECK_THROWS(CircuitConfig::fromFile("/file/does/not/exist"));

        {  // Missing 'networks'
            auto contents = R"({ "manifest": {} })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }
        {  // Missing 'nodes' subnetwork
            auto contents = R"({ "manifest": {}, "networks":{ "edges":[] } })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }
        {  // Missing 'edges' subnetwork
            auto contents = R"({ "manifest": {}, "networks":{ "nodes":[] } })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // Self recursion
            auto contents = R"({
              "manifest": { "$DIR": "$DIR" },
              "networks": {}
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // mutual recursion
            auto contents = R"({
              "manifest": {
                "$FOO": "$BAR",
                "$BAR": "$FOO"
              },
              "networks": {}
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // Invalid variable name
            auto contents = R"({
              "manifest": {
                "$FOO[]": "InvalidVariableName"
              },
              "networks": {}
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // Duplicate population names
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data"
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5"
                  },
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5"
                  }
                ]
              }
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // Biophysical population without morphology dir
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data"
              },
              "components": {
                "morphologies_dir": ""
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5",
                    "populations": {
                      "nodes-A": {
                        "type": "biophysical",
                        "biophysical_neuron_models_dir": "a/fake/dir"
                      },
                      "nodes-B": {
                        "type": "virtualnode"
                      }
                    }
                  }
                ],
                "edges":[]
              }
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // Biophysical population without `biophysical_neuron_models_dir`
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data"
              },
              "components": {
                "morphologies_dir": "a/fake/dir"
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5",
                    "populations": {
                      "nodes-A": {
                        "type": "biophysical"
                      },
                      "nodes-B": {
                        "type": "virtualnode"
                      }
                    }
                  }
                ],
                "edges":[]
              }
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }

        {  // No node file defined for node subnetwork
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data"
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": ""
                  }
                ]
              }
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }
    }

    SECTION("Overrides") {
        {  // Population without overriden properties return default component values
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data",
                "$COMPONENT_DIR": "./"
              },
              "components": {
                "morphologies_dir": "$COMPONENT_DIR/morphologies",
                "biophysical_neuron_models_dir": "$COMPONENT_DIR/biophysical_neuron_models",
                "alternate_morphologies": {
                  "h5v1": "$COMPONENT_DIR/morphologies/h5"
                }
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5",
                    "populations": {
                      "nodes-A": {}
                    }
                  }
                ],
                "edges":[]
              }
            })";
            CHECK(contains(CircuitConfig(contents, "./")
                           .getNodePopulationProperties("nodes-A")
                           .alternateMorphologyFormats["h5v1"],
                           "morphologies/h5"));
        }

        {  // Node population with overriden properties return correct information
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data",
                "$COMPONENT_DIR": "./"
              },
              "components": {
                "morphologies_dir": "$COMPONENT_DIR/morphologies",
                "biophysical_neuron_models_dir": "$COMPONENT_DIR/biophysical_neuron_models",
                "alternate_morphologies": {
                  "h5v1": "$COMPONENT_DIR/morphologies/h5"
                }
              },
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5",
                    "populations": {
                      "nodes-A": {
                        "morphologies_dir": "my/custom/morphologies/dir",
                        "alternate_morphologies": {
                          "h5v1" : "another/custom/morphologies/dir"
                        }
                      },
                      "nodes-B": {
                        "alternate_morphologies": {
                          "h5v1" : "another/custom/morphologies/dir"
                        }
                      }
                    }
                  }
                ],
                "edges":[]
              }
            })";
            CHECK(contains(CircuitConfig(contents, "./")
                           .getNodePopulationProperties("nodes-A")
                           .morphologiesDir,
                           "/my/custom/morphologies/dir"));
        }

        {  // Edge population with overriden properties return correct information
            auto contents = R"({
              "manifest": {
                "$NETWORK_DIR": "./data",
                "$COMPONENT_DIR": "./"
              },
              "components": {
                "morphologies_dir": "$COMPONENT_DIR/morphologies",
                "biophysical_neuron_models_dir": "$COMPONENT_DIR/biophysical_neuron_models",
                "alternate_morphologies": {
                  "h5v1": "$COMPONENT_DIR/morphologies/h5"
                }
              },
              "networks": {
                "edges": [
                  {
                    "edges_file": "$NETWORK_DIR/edges1.h5",
                    "populations": {
                      "edges-AB": {
                        "morphologies_dir": "my/custom/morphologies/dir"
                      }
                    }
                  }
                ],
                "nodes":[]
              }
            })";
            CHECK(contains(CircuitConfig(contents, "./")
                           .getEdgePopulationProperties("edges-AB")
                           .morphologiesDir,
                           "my/custom/morphologies/dir"));
        }
    }
}

TEST_CASE("SimulationConfig") {
    SECTION("Simple") {
        const auto config = SimulationConfig::fromFile("./data/config/simulation_config.json");
        CHECK_NOTHROW(config.getRun());
        using Catch::Matchers::WithinULP;
        CHECK(config.getRun().tstop == 1000);
        CHECK(config.getRun().dt == 0.025);
        CHECK(config.getRun().randomSeed == 201506);
        CHECK(config.getRun().spikeThreshold == -35.5);
        CHECK(config.getRun().integrationMethod ==
              SimulationConfig::Run::IntegrationMethod::crank_nicolson_ion);
        CHECK(config.getRun().stimulusSeed == 111);
        CHECK(config.getRun().ionchannelSeed == 222);
        CHECK(config.getRun().minisSeed == 333);
        CHECK(config.getRun().synapseSeed == 444);

        namespace fs = ghc::filesystem;
        const auto basePath = fs::absolute(
            fs::path("./data/config/simulation_config.json").parent_path());

        const auto electrodesPath = fs::absolute(basePath / "electrodes/electrode_weights.h5");
        CHECK(config.getRun().electrodesFile == (electrodesPath).lexically_normal());

        CHECK_NOTHROW(config.getOutput());
        const auto outputPath = fs::absolute(basePath / "some/path/output");
        CHECK(config.getOutput().outputDir == (outputPath).lexically_normal());
        CHECK(config.getOutput().spikesFile == "out.h5");
        CHECK(config.getOutput().logFile.empty());
        CHECK(config.getOutput().sortOrder == SimulationConfig::Output::SpikesSortOrder::by_id);

        CHECK_NOTHROW(config.getConditions());
        CHECK(config.getConditions().celsius == 35.);
        CHECK(config.getConditions().vInit == -80);
        CHECK(config.getConditions().spikeLocation ==
              SimulationConfig::Conditions::SpikeLocation::AIS);
        CHECK(config.getConditions().extracellularCalcium == nonstd::nullopt);
        CHECK(config.getConditions().randomizeGabaRiseTime == false);
        CHECK(config.getConditions().mechanisms.size() == 2);
        auto itr = config.getConditions().mechanisms.find("ProbAMPANMDA_EMS");
        CHECK(itr != config.getConditions().mechanisms.end());
        CHECK(nonstd::get<bool>(itr->second.find("property1")->second) == false);
        CHECK(nonstd::get<int>(itr->second.find("property2")->second) == -1);
        itr = config.getConditions().mechanisms.find("GluSynapse");
        CHECK(nonstd::get<double>(itr->second.find("property3")->second) == 0.025);
        CHECK(nonstd::get<std::string>(itr->second.find("property4")->second) == "test");
        const auto modifications = config.getConditions().getModifications();
        CHECK(modifications.size() == 2);
        const auto TTX = nonstd::get<SimulationConfig::ModificationTTX>(modifications[0]);
        CHECK(TTX.name == "applyTTX");
        CHECK(TTX.type == SimulationConfig::ModificationBase::ModificationType::TTX);
        CHECK(TTX.nodeSet == "single");
        const auto configAllSects = nonstd::get<SimulationConfig::ModificationConfigureAllSections>(modifications[1]);
        CHECK(configAllSects.name == "no_SK_E2");
        CHECK(configAllSects.type ==
              SimulationConfig::ModificationBase::ModificationType::ConfigureAllSections);
        CHECK(configAllSects.sectionConfigure == "%s.gSK_E2bar_SK_E2 = 0");

        CHECK_THROWS_AS(config.getReport("DoesNotExist"), SonataError);

        CHECK(config.listReportNames() == std::set<std::string>{
              "axonal_comp_centers",
              "cell_imembrane",
              "compartment",
              "soma",
              "lfp"
              });

        CHECK(config.getReport("soma").cells == "Column");
        CHECK(config.getReport("soma").type == SimulationConfig::Report::Type::compartment);
        CHECK(config.getReport("soma").compartments == SimulationConfig::Report::Compartments::center);
        CHECK(config.getReport("soma").enabled == true);
        const auto somaFilePath = fs::absolute(config.getOutput().outputDir / fs::path("soma.h5"));
        CHECK(config.getReport("soma").fileName == somaFilePath.lexically_normal());
        CHECK(config.getReport("compartment").dt == 0.1);
        CHECK(config.getReport("compartment").sections == SimulationConfig::Report::Sections::all);
        CHECK(config.getReport("compartment").compartments == SimulationConfig::Report::Compartments::all);
        CHECK(config.getReport("compartment").enabled == false);
        CHECK(config.getReport("axonal_comp_centers").startTime == 0.);
        CHECK(config.getReport("axonal_comp_centers").compartments == SimulationConfig::Report::Compartments::center);
        CHECK(config.getReport("axonal_comp_centers").scaling == SimulationConfig::Report::Scaling::none);
        const auto axonalFilePath = fs::absolute(config.getOutput().outputDir / fs::path("axon_centers.h5"));
        CHECK(config.getReport("axonal_comp_centers").fileName ==
              axonalFilePath.lexically_normal());
        CHECK(config.getReport("cell_imembrane").endTime == 500.);
        CHECK(config.getReport("cell_imembrane").variableName == "i_membrane, IClamp");
        CHECK(config.getReport("lfp").type == SimulationConfig::Report::Type::lfp);

        CHECK_NOTHROW(nlohmann::json::parse(config.getExpandedJSON()));
        CHECK(config.getBasePath() == basePath.lexically_normal());

        const auto network = fs::absolute(basePath / fs::path("circuit_config.json"));
        CHECK(config.getNetwork() == network.lexically_normal());
        CHECK(config.getTargetSimulator() == SimulationConfig::SimulatorType::CORENEURON);
        const auto circuit_conf = CircuitConfig::fromFile(config.getNetwork());
        CHECK(config.getNodeSetsFile() == circuit_conf.getNodeSetsPath());
        CHECK(config.getNodeSet() == "Column");

        using InputType = SimulationConfig::InputBase::InputType;
        using Module = SimulationConfig::InputBase::Module;
        {
            const auto input = nonstd::get<SimulationConfig::InputLinear>(config.getInput("ex_linear"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::linear);
            CHECK(input.delay == 0);
            CHECK(input.duration == 15);
            CHECK(input.nodeSet == "Column");
            CHECK(input.ampStart == 0.15);
            CHECK(input.ampEnd == 0.15);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputRelativeLinear>(config.getInput("ex_rel_linear"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::relative_linear);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000);
            CHECK(input.nodeSet == "Column");
            CHECK(input.percentStart == 80);
            CHECK(input.percentEnd == 20);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputPulse>(config.getInput("ex_pulse"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::pulse);
            CHECK(input.delay == 10);
            CHECK(input.duration == 80);
            CHECK(input.nodeSet == "Mosaic");

            CHECK(input.frequency == 80);
            CHECK(input.ampStart == 2);
            CHECK(input.width == 1);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSinusoidal>(config.getInput("ex_sinusoidal"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::sinusoidal);
            CHECK(input.delay == 10);
            CHECK(input.duration == 80);
            CHECK(input.nodeSet == "Mosaic");

            CHECK(input.frequency == 8);
            CHECK(input.ampStart == 0.2);
            CHECK(input.dt == 0.5);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSinusoidal>(config.getInput("ex_sinusoidal_default_dt"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::sinusoidal);
            CHECK(input.delay == 10);
            CHECK(input.duration == 80);
            CHECK(input.nodeSet == "Mosaic");

            CHECK(input.frequency == 80);
            CHECK(input.ampStart == 2);
            CHECK(input.dt == 0.025);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSubthreshold>(config.getInput("ex_subthreshold"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::subthreshold);
            CHECK(input.delay == 10);
            CHECK(input.duration == 80);
            CHECK(input.nodeSet == "Mosaic");
            CHECK(input.percentLess == 80);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputNoise>(config.getInput("ex_noise_meanpercent"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::noise);
            CHECK(input.delay == 0);
            CHECK(input.duration == 5000);
            CHECK(input.nodeSet == "Rt_RC");
            CHECK(input.meanPercent.value() == 0.01);
            CHECK(input.variance == 0.001);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputNoise>(config.getInput("ex_noise_mean"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::noise);
            CHECK(input.delay == 0);
            CHECK(input.duration == 5000);
            CHECK(input.nodeSet == "Rt_RC");
            CHECK(input.mean.value() == 0);
            CHECK(input.variance == 0.001);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputShotNoise>(config.getInput("ex_shotnoise"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::shot_noise);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000);
            CHECK(input.nodeSet == "L5E");
            CHECK(input.ampMean == 70);
            CHECK(input.ampVar == 40);
            CHECK(input.rate == 4);
            CHECK(input.randomSeed == nonstd::nullopt);
            CHECK(input.riseTime == 0.4);
            CHECK(input.decayTime == 4);
            CHECK(input.reversal == 10);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputRelativeShotNoise>(config.getInput("ex_rel_shotnoise"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::relative_shot_noise);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000);
            CHECK(input.nodeSet == "L5E");
            CHECK(input.meanPercent == 70);
            CHECK(input.sdPercent == 40);
            CHECK(input.randomSeed == 230522);
            CHECK(input.riseTime == 0.4);
            CHECK(input.decayTime == 4);
            CHECK(input.reversal == 0);
            CHECK(input.relativeSkew == 0.5);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputHyperpolarizing>(config.getInput("ex_hyperpolarizing"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::hyperpolarizing);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000);
            CHECK(input.nodeSet == "L5E");
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSynapseReplay>(config.getInput("ex_replay"));
            CHECK(input.inputType == InputType::spikes);
            CHECK(input.module == Module::synapse_replay);
            CHECK(input.delay == 0);
            CHECK(input.duration == 40000);
            CHECK(input.nodeSet == "Column");
            CHECK(endswith(input.spikeFile, "replay.h5"));
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSeclamp>(config.getInput("ex_seclamp"));
            CHECK(input.inputType == InputType::voltage_clamp);
            CHECK(input.module == Module::seclamp);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000.);
            CHECK(input.nodeSet == "L5E");
            CHECK(input.voltage == 1.1);
            CHECK(input.seriesResistance == 0.5);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputAbsoluteShotNoise>(
                config.getInput("ex_abs_shotnoise"));
            CHECK(input.inputType == InputType::conductance);
            CHECK(input.module == Module::absolute_shot_noise);
            CHECK(input.mean == 50);
            CHECK(input.sigma == 5);
            CHECK(input.reversal == 10);
            CHECK(input.randomSeed == nonstd::nullopt);
            CHECK(input.representsPhysicalElectrode == true);
            CHECK(input.relativeSkew == 0.1);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputOrnsteinUhlenbeck>(
                config.getInput("ex_OU"));
            CHECK(input.inputType == InputType::conductance);
            CHECK(input.module == Module::ornstein_uhlenbeck);
            CHECK(input.tau == 2.8);
            CHECK(input.reversal == 10);
            CHECK(input.mean == 50);
            CHECK(input.sigma == 5);
            CHECK(input.randomSeed == nonstd::nullopt);
            CHECK(input.representsPhysicalElectrode == false);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputRelativeOrnsteinUhlenbeck>(
                config.getInput("ex_rel_OU"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::relative_ornstein_uhlenbeck);
            CHECK(input.tau == 2.8);
            CHECK(input.reversal == 0);
            CHECK(input.meanPercent == 70);
            CHECK(input.sdPercent == 10);
            CHECK(input.randomSeed == 230522);
        }

        CHECK(config.listInputNames() == std::set<std::string>{"ex_abs_shotnoise",
                                                               "ex_extracellular_stimulation",
                                                               "ex_hyperpolarizing",
                                                               "ex_linear",
                                                               "ex_noise_mean",
                                                               "ex_noise_meanpercent",
                                                               "ex_OU",
                                                               "ex_pulse",
                                                               "ex_rel_linear",
                                                               "ex_rel_OU",
                                                               "ex_rel_shotnoise",
                                                               "ex_replay",
                                                               "ex_seclamp",
                                                               "ex_shotnoise",
                                                               "ex_sinusoidal",
                                                               "ex_sinusoidal_default_dt",
                                                               "ex_subthreshold"});

        auto overrides = config.getConnectionOverrides();
        CHECK(overrides[0].name == "ConL3Exc-Uni");
        CHECK(overrides[0].source == "Excitatory");
        CHECK(overrides[0].target == "Mosaic");
        CHECK(overrides[0].weight == 1);
        CHECK(overrides[0].spontMinis == 0.01);
        CHECK(overrides[0].modoverride == "GluSynapse");
        CHECK(overrides[0].delay == 0.5);
        CHECK(overrides[0].synapseDelayOverride == nonstd::nullopt);
        CHECK(overrides[0].synapseConfigure == nonstd::nullopt);
        CHECK(overrides[0].neuromodulationDtc == nonstd::nullopt);
        CHECK(overrides[0].neuromodulationStrength == nonstd::nullopt);

        CHECK(overrides[1].name == "GABAB_erev");
        CHECK(overrides[1].spontMinis == nonstd::nullopt);
        CHECK(overrides[1].synapseDelayOverride == 0.5);
        CHECK(overrides[1].delay == 0);
        CHECK(overrides[1].synapseConfigure == "%s.e_GABAA = -82.0 tau_d_GABAB_ProbGABAAB_EMS = 77");
        CHECK(overrides[1].modoverride == nonstd::nullopt);
        CHECK(overrides[1].neuromodulationDtc == 100);
        CHECK(overrides[1].neuromodulationStrength == 0.75);
        REQUIRE_THAT(config.getMetaData(),
                     Catch::Matchers::Predicate<decltype(config.getMetaData())>(
                         [](const auto& metadata) -> bool {
                             return metadata.size() == 4 &&
                                    nonstd::get<std::string>(metadata.at("note")) ==
                                        "first attempt of simulation" &&
                                    nonstd::get<int>(metadata.at("sim_version")) == 1 &&
                                    nonstd::get<double>(metadata.at("v_float")) == 0.5 &&
                                    nonstd::get<bool>(metadata.at("v_bool")) == false;
                         },
                         "metadata matches"));
        REQUIRE_THAT(config.getBetaFeatures(),
                     Catch::Matchers::Predicate<decltype(config.getBetaFeatures())>(
                         [](const auto& features) -> bool {
                             return features.size() == 4 &&
                                    nonstd::get<std::string>(features.at("v_str")) == "abcd" &&
                                    nonstd::get<int>(features.at("v_int")) == 10 &&
                                    nonstd::get<double>(features.at("v_float")) == 0.5 &&
                                    nonstd::get<bool>(features.at("v_bool")) == false;
                         },
                         "beta features match"));
    }

    SECTION("manifest_network_run") {
        auto contents = R"({
          "manifest": {
            "$CIRCUIT_DIR": "./circuit"
          },
          "network": "$CIRCUIT_DIR/circuit_config.json",
          "run": {
            "random_seed": 12345,
            "dt": 0.05,
            "tstop": 1000
          }
        })";
        namespace fs = ghc::filesystem;
        const auto basePath = fs::absolute(fs::path("./").parent_path());
        const auto config = SimulationConfig(contents, basePath);
        const auto network = fs::absolute(basePath / "circuit" / fs::path("circuit_config.json"));
        CHECK(config.getNetwork() == network.lexically_normal());
        CHECK(config.getTargetSimulator() == SimulationConfig::SimulatorType::NEURON);  // default
        CHECK(config.getNodeSetsFile() == "");  // network file is not readable so default empty
        CHECK(config.getNodeSet() == nonstd::nullopt);  // default
        CHECK(config.getRun().stimulusSeed == 0);
        CHECK(config.getRun().ionchannelSeed == 0);
        CHECK(config.getRun().minisSeed == 0);
        CHECK(config.getRun().synapseSeed == 0);
        CHECK(config.getRun().electrodesFile == "");
        CHECK(config.getRun().spikeThreshold == -30.0);
    }

    SECTION("Exception") {
        {  // No run section
            auto contents = R"({})";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No tstop in run section
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No dt in run section
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "tstop": 1000
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No random_seed in run section
            auto contents = R"({
              "run": {
                "tstop": 1000,
                "dt": 0.05
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong spike_location in a condition object
            auto contents = R"({
              "run": {
                "tstop": 1000,
                "dt": 0.05,
                "random_seed": 12345
              },
              "conditions": {
                "spike_location": "axon"
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong integration_method in a run object
            auto contents = R"({
              "run": {
                "tstop": 1000,
                "dt": 0.05,
                "random_seed": 12345,
                "integration_method": 4
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No reports section
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              }
            })";
            CHECK_NOTHROW(SimulationConfig(contents, "./"));
        }
        {  // No cells in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "type": "typestring",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No type in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No variable_name in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "compartment",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong variable_name in a report object: no variable after ','
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "variable_name": "variablestring,",
                   "type": "compartment",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong variable_name in a report object: leading '.'
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "variable_name": ".V_M, V",
                   "type": "compartment",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong variable_name in a report object: more than one '.' in a name
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "variable_name": "AdEx..foo, V",
                   "type": "compartment",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong variable_name in a report object: more than one '.' in a name
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "variable_name": "AdEx.V_M.foo, V",
                   "type": "compartment",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No dt in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "compartment",
                   "variable_name": "variablestring",
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No start_time in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "summation",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No end_time in a report object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "summation",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid sections in a report object
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "sections": "none",
                   "type": "synapse",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid type in a report object, non-existing enum value
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "not-a-valid-type",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid type in a report object, numeric value
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": 0,
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }{  // Invalid type in a report object; `true` value
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": true,
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid scaling in a report object
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "scaling": "linear",
                   "type": "compartment",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid compartments in a report object
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "compartments": "middle",
                   "type": "compartment",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0,
                   "end_time": 500
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // wrong input_type in an input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "linear": {
                   "input_type": "current",
                   "module": "linear",
                   "amp_start": 0.15,
                   "delay": 0,
                   "duration": 15,
                   "node_set":"Column"
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // wrong module in an input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "linear": {
                   "input_type": "current_clamp",
                   "module": "spike_replay",
                   "amp_start": 0.15,
                   "delay": 0,
                   "duration": 15,
                   "node_set":"Column"
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // wrong input_type for module in an input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                  "ex_hyperpolarizing": {
                      "input_type": "voltage_clamp",
                      "module": "hyperpolarizing",
                      "delay": 0,
                      "duration": 1000,
                      "node_set": "L5E"
                  }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // no amp_start in a linear input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "linear": {
                   "input_type": "current_clamp",
                   "module": "spike_replay",
                   "delay": 0,
                   "duration": 15,
                   "node_set":"Column"
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Both mean and mean_percent are given in a noise input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "noise": {
                   "input_type": "current_clamp",
                   "module": "noise",
                   "delay": 0,
                   "duration": 15,
                   "node_set":"Column",
                   "mean" : 0.1,
                   "mean_percent": 0.01,
                   "variance": 0.001
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No valuable mean or mean_percent are given in a noise input object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "noise": {
                   "input_type": "current_clamp",
                   "module": "noise",
                   "delay": 0,
                   "duration": 15,
                   "node_set":"Column",
                   "variance": 0.001,
                   "mean" : null
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        { // mising input
            const auto config = SimulationConfig::fromFile("./data/config/simulation_config.json");
            CHECK_THROWS_AS(config.getInput("does_not_exist"), SonataError);
        }
        {  // non-existant input_type
            const auto* contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs": {
                "noise": {
                   "input_type": "Does_not_exist",
                   "module": "hyperpolarization"
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Invalid spikes ordering in the output section
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "output": {
                "output_dir": "$OUTPUT_DIR/output",
                "spikes_file": "out.h5",
                "spikes_sort_order": "invalid-order"
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {
            // Missing mandatory parameter in a connection_override object
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "connection_overrides": {
                "connect1": {
                   "source": "Excitatory",
                   "weight": 1
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // Wrong simulator type
            auto contents = R"({
              "target_simulator" : "BLABLA",
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {
            // Replay with a non-h5 file
            auto contents = R"({
              "run": {
                "random_seed": 12345,
                "dt": 0.05,
                "tstop": 1000
              },
              "inputs" : {
                "ex_replay": {
                    "input_type": "spikes",
                    "module": "synapse_replay",
                    "delay": 0.0,
                    "duration": 40000.0,
                    "spike_file": "replay.dat",
                    "node_set": "Column"
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
    }
}
