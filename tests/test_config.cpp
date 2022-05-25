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
        CHECK(config.getEdgePopulationProperties("edges-AB").type == "chemical_synapse");

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

        {  // Biophysical population with morphology dir
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
                        "morphologies_dir": "/a/custom/morphology/path"
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
            CHECK_NOTHROW(CircuitConfig(contents, "./"));
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
                        "morphologies_dir": "my/custom/morpholgoies/dir",
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
                           "/my/custom/morpholgoies/dir"));
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
                        "morphologies_dir": "my/custom/morpholgoies/dir"
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
                           "/my/custom/morpholgoies/dir"));
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

        namespace fs = ghc::filesystem;
        const auto basePath = fs::absolute(
            fs::path("./data/config/simulation_config.json").parent_path());

        CHECK_NOTHROW(config.getOutput());
        const auto outputPath = fs::absolute(basePath / fs::path("output"));
        CHECK(config.getOutput().outputDir == outputPath.lexically_normal());
        CHECK(config.getOutput().spikesFile == "out.h5");

        CHECK_THROWS_AS(config.getReport("DoesNotExist"), SonataError);

        CHECK(config.getReport("soma").cells == "Mosaic");
        CHECK(config.getReport("soma").type == SimulationConfig::Report::Type::compartment);
        CHECK(config.getReport("soma").compartments == SimulationConfig::Report::Compartments::center);
        CHECK(config.getReport("soma").enabled == true);
        CHECK(config.getReport("compartment").dt == 0.1);
        CHECK(config.getReport("compartment").sections == SimulationConfig::Report::Sections::all);
        CHECK(config.getReport("compartment").compartments == SimulationConfig::Report::Compartments::all);
        CHECK(config.getReport("compartment").enabled == false);
        CHECK(config.getReport("axonal_comp_centers").startTime == 0.);
        CHECK(config.getReport("axonal_comp_centers").compartments == SimulationConfig::Report::Compartments::center);
        CHECK(config.getReport("axonal_comp_centers").scaling == SimulationConfig::Report::Scaling::none);
        const auto axonalFilePath = fs::absolute(basePath / fs::path("axon_centers.h5"));
        CHECK(config.getReport("axonal_comp_centers").fileName ==
              axonalFilePath.lexically_normal());
        CHECK(config.getReport("cell_imembrane").endTime == 500.);
        CHECK(config.getReport("cell_imembrane").variableName == "i_membrane, IClamp");

        CHECK_NOTHROW(nlohmann::json::parse(config.getJSON()));
        CHECK(config.getBasePath() == basePath.lexically_normal());

        const auto network = fs::absolute(basePath / fs::path("circuit_config.json"));
        CHECK(config.getNetwork() == network.lexically_normal());

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
            CHECK(input.randomSeed == 201506);
            CHECK(input.riseTime == 0.4);
            CHECK(input.decayTime == 4);
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputRelativeShotNoise>(config.getInput("ex_rel_shotnoise"));
            CHECK(input.inputType == InputType::current_clamp);
            CHECK(input.module == Module::relative_shot_noise);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000);
            CHECK(input.nodeSet == "L5E");
            CHECK(input.ampCv == 0.63);
            CHECK(input.meanPercent == 70);
            CHECK(input.sdPercent == 40);
            CHECK(input.randomSeed == 201506);
            CHECK(input.riseTime == 0.4);
            CHECK(input.decayTime == 4);
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
            CHECK(endswith(input.spikeFile, "replay.dat"));
            CHECK(input.source == "ML_afferents");
        }
        {
            const auto input = nonstd::get<SimulationConfig::InputSeclamp>(config.getInput("ex_seclamp"));
            CHECK(input.inputType == InputType::voltage_clamp);
            CHECK(input.module == Module::seclamp);
            CHECK(input.delay == 0);
            CHECK(input.duration == 1000.);
            CHECK(input.nodeSet == "L5E");
            CHECK(input.voltage == 1.1);
        }
    }
    SECTION("manifest_network") {
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
        {  // Wrong variable_name in a report object
            auto contents = R"({
              "run": {
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
        {  // No mean or mean_percent are given in a noise input object
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
                   "variance": 0.001
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
    }
}
