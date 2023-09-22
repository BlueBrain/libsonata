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

    SECTION("CompoundRecursiveMissing") {
        auto node_sets = R"({"compound0": ["compound1"], "compound1": ["missing"] })";
        CHECK_THROWS_AS(NodeSets{node_sets}, SonataError);
    }

    SECTION("MissingNameInMaterialize") {
        const NodePopulation population("./data/nodes1.h5", "", "nodes-A");
        auto node_sets = R"({ "NodeSet0": { "attr-Y": 21 } })";
        NodeSets ns(node_sets);
        CHECK_THROWS_AS(ns.materialize("NONEXISTANT", population), SonataError);
    }

    SECTION("OperatorMultipleClauses")
    {
        auto node_sets = R""({ "NodeSet0": {"attr-Y": {"has to be ops": 3, "2nd": 3}} })"";
        CHECK_THROWS_AS(NodeSets(node_sets), SonataError);
    }

    SECTION("OperatorObject")
    {
        auto node_sets = R""({ "NodeSet0": {"attr-Y": {"has to be ops": {}}} })"";
        CHECK_THROWS_AS(NodeSets(node_sets), SonataError);
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

    SECTION("BasicScalarOperatorStringRegex") {
        {
            auto node_sets = R"({ "NodeSet0": {"E-mapping-good": {"$regex": "^[AC].*"}} })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{0, 1}, {2, 6}}));
        }

        {
            auto node_sets = R""({ "NodeSet0": {"attr-Z": {"$regex": "^(aa|bb|ff)"}} })"";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{0, 2}, {5,6}}));
        }
        {
            auto node_sets = R""({ "NodeSet0": {"attr-Z": {"$op-does-not-exist": "dne"}} })"";
            CHECK_THROWS_AS(NodeSets(node_sets), SonataError);
        }
    }

    SECTION("BasicScalarOperatorNumeric") {
        {
            auto node_sets = R"({ "NodeSet0": {"attr-Y": {"$gt": 23}} })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{3, 6}}));
        }
        {
            auto node_sets = R"({ "NodeSet0": {"attr-Y": {"$lt": 23}} })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{0, 2}}));
        }
        {
            auto node_sets = R"({ "NodeSet0": {"attr-Y": {"$gte": 23}} })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{2, 6}}));
        }
        {
            auto node_sets = R"({ "NodeSet0": {"attr-Y": {"$lte": 23}} })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({{0, 3}}));
        }
        {
            auto node_sets = R""({ "NodeSet0": {"attr-Y": {"$op-does-not-exist": 3}} })"";
            CHECK_THROWS_AS(NodeSets(node_sets), SonataError);
        }
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
            auto node_sets = R"({ "NodeSet0": { "node_id": [10000, 20000] } })";
            NodeSets ns(node_sets);
            Selection sel = ns.materialize("NodeSet0", population);
            CHECK(sel == Selection({}));
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
            auto node_sets = R"({ "NodeSet0": { "population": ["nodes-A", "FAKE"] } })";
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

    SECTION("EmptyArray")
    {
        auto node_sets = R""({ "NodeSet0": {"node_id": [] } })"";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSet0", population);
        CHECK(sel == Selection({}));
    }
}

TEST_CASE("NodeSetCompound") {
    const NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    SECTION("Compound") {
        {
            const auto *const node_sets = R"({
                "NodeSet0": { "node_id": [1] },
                "NodeSet1": { "node_id": [2] },
                "NodeSetCompound0": ["NodeSet0", "NodeSet1"],
                "NodeSetCompound1": ["NodeSetCompound0", "NodeSet2"],
                "NodeSetCompound2": ["NodeSetCompound1"],
                "NodeSetCompound3": ["NodeSetCompound2"],
                "NodeSetCompound4": ["NodeSetCompound3"],
                "NodeSetCompound5": ["NodeSetCompound4"],
                "NodeSet2": { "node_id": [3] }
            })";
            NodeSets ns(node_sets);
            Selection expected{{{1, 4}}};
            CHECK(expected == ns.materialize("NodeSetCompound1", population));
            CHECK(expected == ns.materialize("NodeSetCompound2", population));
            CHECK(expected == ns.materialize("NodeSetCompound3", population));
            CHECK(expected == ns.materialize("NodeSetCompound4", population));
            CHECK(expected == ns.materialize("NodeSetCompound5", population));
        }

        {
            const auto *const node_sets = R"({
                "NodeSet0": { "E-mapping-good": ["A", "B"] },
                "NodeSet1": { "attr-Y": 21 },
                "NodeSetCompound0": ["NodeSet0", "NodeSet1"],
                "NodeSetCompound1": ["NodeSetCompound0"],
                "NodeSetCompound2": ["NodeSetCompound1"],
                "NodeSetCompound3": ["NodeSetCompound2"],
                "NodeSetCompound4": ["NodeSetCompound3"],
                "NodeSetCompound5": ["NodeSetCompound4"]
            })";
            NodeSets ns(node_sets);
            Selection expected{{{0, 2}, {3, 4}}};
            CHECK(expected == ns.materialize("NodeSetCompound1", population));
            CHECK(expected == ns.materialize("NodeSetCompound2", population));
            CHECK(expected == ns.materialize("NodeSetCompound3", population));
            CHECK(expected == ns.materialize("NodeSetCompound4", population));
            CHECK(expected == ns.materialize("NodeSetCompound5", population));
        }
    }

    SECTION("EmptyCompoundArray")
    {
        auto node_sets = R""({ "NodeSet0": {"node_id": [] },
                               "NodeSetCompound0": []
                }
        )"";
        NodeSets ns(node_sets);
        Selection sel = ns.materialize("NodeSetCompound0", population);
        CHECK(sel == Selection({}));
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
        "power_number_test": {
            "numeric_attribute_gt": { "$gt": 3 },
            "numeric_attribute_lt": { "$lt": 3 },
            "numeric_attribute_gte": { "$gte": 3 },
            "numeric_attribute_lte": { "$lte": 3 }
        },
        "power_regex_test": {
            "string_attr": { "$regex": "^[s][o]me value$" }
        },
        "combined": ["bio_layer45", "V1_point_prime"]
    })";

    SECTION("toJSON") {
        {
            NodeSets ns0(node_sets);
            std::string j = ns0.toJSON();
            NodeSets ns1(j);
            CHECK(ns0.toJSON() == ns1.toJSON());

            auto ns = NodeSets::fromFile("./data/node_sets.json");
            CHECK(ns.toJSON() == ns1.toJSON());
        }

        {
            NodeSets ns0(R"({"AND": {"node_id": [], "mtype": "L6_Y"}})");
            CHECK(ns0.toJSON() == "{\n  \"AND\": {\"mtype\": [\"L6_Y\"] }\n}");
        }

        {
            NodeSets ns0(R"({"NAME": {"node_id": []}})");
            CHECK(ns0.toJSON() == "{\n}");

            NodeSets ns1(R"({})");
            CHECK(ns1.toJSON() == "{\n}");
        }
    }

    SECTION("names") {
        NodeSets ns(node_sets);
        std::set<std::string> expected = {"bio_layer45", "V1_point_prime", "combined", "power_number_test", "power_regex_test"};
        CHECK(ns.names() == expected);
    }
}
