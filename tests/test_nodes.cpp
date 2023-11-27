#include <catch2/catch.hpp>

#include <bbp/sonata/nodes.h>

#include <iostream>
#include <string>
#include <vector>


using namespace bbp::sonata;


TEST_CASE("NodePopulation", "[base]") {
    const NodePopulation population("./data/nodes1.h5", "", "nodes-A");

    CHECK(population.name() == "nodes-A");

    REQUIRE(population.size() == 6);

    REQUIRE(population.attributeNames() == std::set<std::string>{"attr-X",
                                                                 "attr-Y",
                                                                 "attr-Z",
                                                                 "A-double",
                                                                 "A-float",
                                                                 "A-int64",
                                                                 "A-int32",
                                                                 "A-int16",
                                                                 "A-int8",
                                                                 "A-uint64",
                                                                 "A-uint32",
                                                                 "A-uint16",
                                                                 "A-uint8",
                                                                 "A-string",
                                                                 "A-enum",
                                                                 "E-mapping-good",
                                                                 "E-mapping-bad"});

    REQUIRE(population.enumerationNames() ==
            std::set<std::string>{"E-mapping-good", "E-mapping-bad"});

    CHECK(population.getAttribute<double>("attr-X", Selection({})) ==
          std::vector<double>{});
    CHECK(population.getAttribute<double>("attr-X", Selection({{0, 1}, {5, 6}})) ==
          std::vector<double>{11.0, 16.0});
    CHECK(population.getAttribute<float>("attr-X", Selection({{0, 1}})) ==
          std::vector<float>{11.0f});
    CHECK(population.getAttribute<uint64_t>("attr-Y", Selection({{0, 1}, {5, 6}})) ==
          std::vector<uint64_t>{21, 26});
    CHECK(population.getAttribute<int64_t>("attr-Y", Selection({{0, 1}})) ==
          std::vector<int64_t>{21});
    CHECK(population.getAttribute<uint32_t>("attr-Y", Selection({{0, 1}})) ==
          std::vector<uint32_t>{21});
    CHECK(population.getAttribute<int32_t>("attr-Y", Selection({{0, 1}})) ==
          std::vector<int32_t>{21});
    CHECK(population.getAttribute<uint8_t>("attr-Y", Selection({{0, 1}})) ==
          std::vector<uint8_t>{21});
    CHECK(population.getAttribute<int8_t>("attr-Y", Selection({{0, 1}})) ==
          std::vector<int8_t>{21});
    CHECK(population.getAttribute<std::string>("attr-Z", Selection({{0, 2}})) ==
          std::vector<std::string>{"aa", "bb"});
    CHECK(population.getAttribute<std::string>("attr-Z", Selection({{0, 1}, {5, 6}})) ==
          std::vector<std::string>{"aa", "ff"});
    CHECK_THROWS_AS(population.getAttribute<double>("no-such-attribute", Selection({{0, 1}})),
                    SonataError);

    // getAttribute with default value
    CHECK(population.getAttribute<double>("attr-X", Selection({{0, 1}, {5, 6}}), 42.0) ==
          std::vector<double>{11.0, 16.0});
    CHECK_THROWS_AS(population.getAttribute<double>("no-such-attribute", Selection({{0, 1}}), 42.0),
                    SonataError);

    CHECK(population._attributeDataType("A-double") == "double");
    CHECK(population._attributeDataType("A-float") == "float");
    CHECK(population._attributeDataType("A-int64") == "int64_t");
    CHECK(population._attributeDataType("A-int32") == "int32_t");
    CHECK(population._attributeDataType("A-int16") == "int16_t");
    CHECK(population._attributeDataType("A-int8") == "int8_t");
    CHECK(population._attributeDataType("A-uint64") == "uint64_t");
    CHECK(population._attributeDataType("A-uint32") == "uint32_t");
    CHECK(population._attributeDataType("A-uint16") == "uint16_t");
    CHECK(population._attributeDataType("A-uint8") == "uint8_t");
    CHECK(population._attributeDataType("A-string") == "string");
    CHECK(population._attributeDataType("E-mapping-good", /* translate_enumeration */ true) ==
          "string");
    CHECK_THROWS_AS(population._attributeDataType("A-enum"), SonataError);
    CHECK_THROWS_AS(population._attributeDataType("no-such-attribute"), SonataError);

    CHECK(population.getEnumeration<size_t>("E-mapping-good", Selection({{0, 1}, {2, 3}})) ==
          std::vector<size_t>{2, 2});
    CHECK_THROWS_AS(population.getEnumeration<size_t>("no-such-enum", Selection({{1, 2}})),
                    SonataError);
    CHECK_THROWS_AS(population.getEnumeration<std::string>("E-mapping-good", Selection({{1, 2}})),
                    SonataError);

    CHECK(population.getAttribute<size_t>("E-mapping-good", Selection({{0, 1}, {2, 3}})) ==
          std::vector<size_t>{2, 2});

    CHECK(population.getAttribute<std::string>("E-mapping-good", Selection({{0, 1}})) ==
          std::vector<std::string>{"C"});

    CHECK_THROWS_AS(population.enumerationValues("no-such-enum"), SonataError);
    CHECK(population.enumerationValues("E-mapping-good") ==
          std::vector<std::string>{"A", "B", "C"});

    CHECK(population.getAttribute<std::string>("E-mapping-bad", Selection({{0, 1}})) ==
          std::vector<std::string>{"C"});

    CHECK_THROWS_AS(population.getAttribute<std::string>("E-mapping-bad", Selection({{1, 2}})),
                    SonataError);

    REQUIRE(population.dynamicsAttributeNames() ==
            std::set<std::string>{"dparam-X", "dparam-Y", "dparam-Z"});

    CHECK(population.getDynamicsAttribute<double>("dparam-X", Selection({{0, 1}, {5, 6}})) ==
          std::vector<double>{1011.0, 1016.0});
    CHECK(population.getDynamicsAttribute<float>("dparam-X", Selection({{0, 1}})) ==
          std::vector<float>{1011.0f});
    CHECK(population.getDynamicsAttribute<uint64_t>("dparam-Y", Selection({{0, 1}, {5, 6}})) ==
          std::vector<uint64_t>{1021, 1026});
    CHECK(population.getDynamicsAttribute<int64_t>("dparam-Y", Selection({{0, 1}})) ==
          std::vector<int64_t>{1021});
    CHECK(population.getDynamicsAttribute<uint32_t>("dparam-Y", Selection({{0, 1}})) ==
          std::vector<uint32_t>{1021});
    CHECK(population.getDynamicsAttribute<int32_t>("dparam-Y", Selection({{0, 1}})) ==
          std::vector<int32_t>{1021});
    CHECK(population.getDynamicsAttribute<uint16_t>("dparam-Y", Selection({{0, 1}})) ==
          std::vector<uint16_t>{1021});
    CHECK(population.getDynamicsAttribute<int16_t>("dparam-Y", Selection({{0, 1}})) ==
          std::vector<int16_t>{1021});
    CHECK(population.getDynamicsAttribute<std::string>("dparam-Z", Selection({{0, 2}})) ==
          std::vector<std::string>{"d-aa", "d-bb"});
    CHECK(population.getDynamicsAttribute<std::string>("dparam-Z", Selection({{0, 1}, {5, 6}})) ==
          std::vector<std::string>{"d-aa", "d-ff"});
    CHECK_THROWS_AS(population.getDynamicsAttribute<double>("no-such-attribute",
                                                            Selection({{0, 1}})),
                    SonataError);

    // getDynamicsAttribute with default value
    CHECK(population.getDynamicsAttribute<double>("dparam-X", Selection({{0, 1}, {5, 6}}), 42.0) ==
          std::vector<double>{1011.0, 1016.0});
    CHECK_THROWS_AS(population.getDynamicsAttribute<double>("no-such-attribute",
                                                            Selection({{0, 1}}),
                                                            42.0),
                    SonataError);

    CHECK(population._dynamicsAttributeDataType("dparam-X") == "double");
    CHECK(population._dynamicsAttributeDataType("dparam-Y") == "int64_t");
    CHECK(population._dynamicsAttributeDataType("dparam-Z") == "string");
}


TEST_CASE("NodePopulationMove", "[base]") {
    NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    NodePopulation pop2 = std::move(population);
    CHECK(pop2.name() == "nodes-A");
}

TEST_CASE("NodePopulationSelectAll", "[base]") {
    NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    CHECK(population.selectAll().flatSize() == 6);
}

TEST_CASE("NodePopulationmatchAttributeValues", "[base]") {
    NodePopulation population("./data/nodes1.h5", "", "nodes-A");

    SECTION("int") {
        const auto sel = population.matchAttributeValues("A-int64", 1);
        CHECK(sel.flatSize() == 0);

        CHECK_THROWS_AS(population.matchAttributeValues("E-mapping-good", 1), SonataError);
    }

    SECTION("String") {
        auto sel = population.matchAttributeValues<std::string>("attr-Z", "bb");
        CHECK(sel.flatSize() == 1);
        CHECK(Selection::fromValues({1}) == sel);

        CHECK_THROWS_AS(population.matchAttributeValues<std::string>("attr-Y", "bb"), SonataError);
    }

    SECTION("Enumeration") {
        auto sel0 = population.matchAttributeValues("E-mapping-good", std::string("C"));
        CHECK(sel0.flatSize() == 4);
        CHECK(Selection::fromValues({0, 2, 4, 5}) == sel0);

        auto sel1 = population.matchAttributeValues("E-mapping-good",
                                                    std::string("does-not-exist"));
        CHECK(Selection({}) == sel1);

        std::vector<std::string> strings {"C", "C", "C", "A", "B", "A"};
        auto sel2 = population.matchAttributeValues("E-mapping-good", strings);
        CHECK(sel2.flatSize() == 6);
        CHECK(Selection({{0, 6}}) == sel2);
    }

    SECTION("Float attribute") {
        CHECK_THROWS_AS(population.matchAttributeValues("attr-X", 2), SonataError);
    }
}

TEMPLATE_TEST_CASE("NodePopulationmatchAttributeValues",
                   "Numeric",
                   int8_t,
                   uint8_t,
                   int16_t,
                   uint16_t,
                   int32_t,
                   uint32_t,
                   int64_t,
                   uint64_t) {
    NodePopulation population("./data/nodes1.h5", "", "nodes-A");
    auto sel = population.matchAttributeValues<TestType>("attr-Y", 23);
    CHECK(sel.flatSize() == 1);
    CHECK(Selection::fromValues({2}) == sel);
}
