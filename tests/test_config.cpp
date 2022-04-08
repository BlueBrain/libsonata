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

bool endswith(const std::string haystack, const std::string needle) {
    return std::equal(needle.rbegin(), needle.rend(), haystack.rbegin());
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
            CHECK(CircuitConfig(contents, "./")
                      .getNodePopulationProperties("nodes-A")
                      .alternateMorphologyFormats["h5v1"]
                      .find("morphologies/h5") != std::string::npos);
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
            CHECK(CircuitConfig(contents, "./")
                      .getNodePopulationProperties("nodes-A")
                      .morphologiesDir.find("/my/custom/morpholgoies/dir") != std::string::npos);
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
            CHECK(CircuitConfig(contents, "./")
                      .getEdgePopulationProperties("edges-AB")
                      .morphologiesDir.find("/my/custom/morpholgoies/dir") != std::string::npos);
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

        namespace fs = ghc::filesystem;
        const auto basePath = fs::absolute(
            fs::path("./data/config/simulation_config.json").parent_path());

        CHECK_NOTHROW(config.getOutput());
        const auto outputPath = fs::absolute(basePath / fs::path("output"));
        CHECK(config.getOutput().outputDir == outputPath.lexically_normal());
        CHECK(config.getOutput().spikesFile == "out.h5");

        CHECK_THROWS_AS(config.getReport("DoesNotExist"), SonataError);

        CHECK(config.getReport("soma").cells == "Mosaic");
        CHECK(config.getReport("soma").type == "compartment");
        CHECK(config.getReport("compartment").dt == 0.1);
        CHECK(config.getReport("axonal_comp_centers").startTime == 0.);
        const auto axonalFilePath = fs::absolute(basePath / fs::path("axon_centers.h5"));
        CHECK(config.getReport("axonal_comp_centers").fileName ==
              axonalFilePath.lexically_normal());
        CHECK(config.getReport("cell_imembrane").endTime == 500.);

        CHECK_NOTHROW(nlohmann::json::parse(config.getJSON()));
        CHECK(config.getBasePath() == basePath.lexically_normal());

        const auto network = fs::absolute(basePath / fs::path("circuit_config.json"));
        CHECK(config.getNetwork() == network.lexically_normal());
    }
    SECTION("manifest_network") {
        auto contents = R"({
          "manifest": {
            "$CIRCUIT_DIR": "./circuit"
          },
          "network": "$CIRCUIT_DIR/circuit_config.json",
          "run": {
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
                "dt": 0.05
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No dt in run section
            auto contents = R"({
              "run": {
                "tstop": 1000
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
        {  // No reports section
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              }
            })";
            CHECK_NOTHROW(SimulationConfig(contents, "./"));
        }
        {  // No cells in a report object
            auto contents = R"({
              "run": {
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
        {  // No dt in a report object
            auto contents = R"({
              "run": {
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "typestring",
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
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "typestring",
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
                "dt": 0.05,
                "tstop": 1000
              },
              "reports": {
                "test": {
                   "cells": "nodesetstring",
                   "type": "typestring",
                   "variable_name": "variablestring",
                   "dt": 0.05,
                   "start_time": 0
                }
              }
            })";
            CHECK_THROWS_AS(SimulationConfig(contents, "./"), SonataError);
        }
    }
}
