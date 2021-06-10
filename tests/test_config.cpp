#include <catch2/catch.hpp>

#include <nlohmann/json.hpp>

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
              "networks": {
                "nodes": [
                  {
                    "nodes_file": "$NETWORK_DIR/nodes1.h5"
                  }
                ]
              }
            })";
            CHECK_THROWS_AS(CircuitConfig(contents, "./"), SonataError);
        }
    }
}
