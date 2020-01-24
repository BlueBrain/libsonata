#include <catch2/catch.hpp>

#include <bbp/sonata/node_sets.h>
#include <bbp/sonata/nodes.h>

using namespace bbp::sonata;

TEST_CASE("NodeSetParse") {
    SECTION("CorrectStructure") {
        CHECK_THROWS_AS(NodeSets{"[ 1 ]"}, SonataError);
        CHECK_THROWS_AS(NodeSets{"1"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "node_id": [{"object": "not_allowed"}] } })"},
                        SonataError);
    }

    SECTION("BasicScalarFailFloat") {
        auto node_sets = R"({ "NodeSet0": { "node_id": 1.5 } })";
        CHECK_THROWS_AS(NodeSets{node_sets}, SonataError);
    }

    SECTION("BasicScalarFailNegativeNodeIds") {
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "node_id": -1 } })"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "node_id": [1, -1] } })"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "node_id": "1" } })"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "node_id": ["1"] } })"}, SonataError);
    }

    SECTION("BasicScalarFailPopulation") {
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "population": 1 } })"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "population": [1, 2] } })"}, SonataError);
        CHECK_THROWS_AS(NodeSets{R"({ "NodeSet0": { "population": ["foo", "bar"] } })"},
                        SonataError);
    }

    SECTION("CompoundNonString") {
        auto node_sets = R"({"compound": ["foo", 1] })";
        CHECK_THROWS_AS(NodeSets{node_sets}, SonataError);
    }

    SECTION("CompoundRecursive") {
        auto node_sets = R"({"compound": ["compound"] })";
        CHECK_THROWS_AS(NodeSets{node_sets}, SonataError);
    }

    SECTION("CompoundMissing") {
        auto node_sets = R"({"compound": ["missing"] })";
        CHECK_THROWS_AS(NodeSets{node_sets}, SonataError);
    }
}

TEST_CASE("NodeSetBasic") {
    const NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    SECTION("BasicScalarInt") {
        auto node_sets = R"({ "NodeSet0": { "attr-Y": 21 } })";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSet0", population);
        CHECK(sel == Selection({
                         {0, 1},
                     }));
    }

    SECTION("BasicScalarString") {
        auto node_sets = R"({ "NodeSet0": { "attr-Z": ["aa", "cc"] } })";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSet0", population);
        CHECK(sel == Selection({{0, 1}, {2, 3}}));
    }

    SECTION("BasicScalarEnum") {
        auto node_sets = R"({ "NodeSet0": { "E-mapping-good": "C" } })";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSet0", population);
        CHECK(sel == Selection({{0, 1}, {2, 3}, {4, 6}}));
    }

    SECTION("BasicScalarAnded") {
        auto node_sets = R"({"NodeSet0": {"E-mapping-good": "C",
                                          "attr-Y": [21, 22]
                                          }
                            })";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSet0", population);
        CHECK(sel == Selection({{0, 1}}));
    }

    SECTION("BasicScalarNodeId") {
        {
            auto node_sets = R"({ "NodeSet0": { "node_id": 1 } })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({
                             {1, 2},
                         }));
        }
        {
            auto node_sets = R"({ "NodeSet0": { "node_id": [1, 3, 5] } })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{1, 2}, {3, 4}, {5, 6}}));
        }
    }

    SECTION("BasicScalarPopulation") {
        {
            auto node_sets = R"({ "NodeSet0": { "population": "nodes-A" } })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == population.selectAll());
        }

        {
            auto node_sets = R"({ "NodeSet0": { "population": "NOT_A_POP" } })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection{{}});
        }
    }
}

TEST_CASE("NodeSetCompound") {
    const NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    SECTION("Compound") {
        auto node_sets = R"({
            "NodeSet0": { "node_id": [1] },
            "NodeSet1": { "node_id": [2] },
            "NodeSetCompound": ["NodeSet0", "NodeSet1", "NodeSet2"],
            "NodeSet2": { "node_id": [3] }
        })";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSetCompound", population);
        CHECK(sel == Selection({{1, 4}}));
    }
}

TEST_CASE("NodeSet") {
    auto node_sets = R"(
    {
        "bio_layer45": {
            "model_type": "biophysical",
            "location": ["layer4", "layer5"]
        },
        "V1_point_prime": {
            "population": "biophysical",
            "model_type": "point",
            "node_id": [1, 2, 3, 5, 7, 9]
        },
        "combined": ["bio_layer45", "V1_point_prime"]
    })";

    SECTION("toJSON") {
        NodeSets ns0(node_sets);
        std::string j = ns0.toJSON();
        NodeSets ns1(j);
        CHECK(ns0.toJSON() == ns1.toJSON());
    }

    SECTION("names") {
        NodeSets ns(node_sets);
        std::set<std::string> expected = {"bio_layer45", "V1_point_prime", "combined"};
        CHECK(ns.names() == expected);
    }
}
