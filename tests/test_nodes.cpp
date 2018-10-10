#include "contrib/catch.hpp"

#include <bbp/sonata/nodes.h>

#include <iostream>
#include <string>
#include <vector>


using namespace bbp::sonata;


TEST_CASE("Selection", "[base]")
{
    SECTION("range check") {
        CHECK_THROWS_AS(
            Selection({{0, 2}, {3, 3}}),
            SonataError
        );
        CHECK_THROWS_AS(
            Selection({{5, 3}}),
            SonataError
        );
    }
    SECTION("fromValues") {
        const auto selection = Selection::fromValues({1, 3, 4, 1});
        CHECK(selection.ranges() == Selection::Ranges{{1, 2}, {3, 5}, {1, 2}});
    }
    SECTION("empty") {
        const auto selection = Selection({});
        CHECK(selection.ranges().empty());
        CHECK(selection.flatten().empty());
        CHECK(selection.flatSize() == 0);
        CHECK(selection.empty());
    }
    SECTION("basic") {
        const Selection::Ranges ranges{{3, 5}, {0, 3}};
        Selection selection(ranges);  // copying ranges

        CHECK(selection.ranges() == ranges);
        CHECK(selection.flatten() == std::vector<EdgeID>{3, 4, 0, 1, 2});
        CHECK(selection.flatSize() == 5);
        CHECK(!selection.empty());
    }
}


TEST_CASE("NodePopulation", "[base]")
{
    const NodePopulation population("./data/nodes1.h5", "", "nodes-A");

    CHECK(population.name() == "nodes-A");

    REQUIRE(population.size() == 5);

    REQUIRE(population.attributeNames() == std::set<std::string>{"attr-X", "attr-Y", "attr-Z"});

    CHECK(
       population.getAttribute<double>("attr-X", Selection({{0, 1}, {5, 6}})) == std::vector<double>{11.0, 16.0}
    );
    CHECK(
       population.getAttribute<float>("attr-X", Selection({{0, 1}})) == std::vector<float>{11.0f}
    );
    CHECK(
       population.getAttribute<uint64_t>("attr-Y", Selection({{0, 1}, {5, 6}})) == std::vector<uint64_t>{21, 26}
    );
    CHECK(
       population.getAttribute<int64_t>("attr-Y", Selection({{0, 1}})) == std::vector<int64_t>{21}
    );
    CHECK(
       population.getAttribute<uint32_t>("attr-Y", Selection({{0, 1}})) == std::vector<uint32_t>{21}
    );
    CHECK(
       population.getAttribute<int32_t>("attr-Y", Selection({{0, 1}})) == std::vector<int32_t>{21}
    );
    CHECK(
       population.getAttribute<uint8_t>("attr-Y", Selection({{0, 1}})) == std::vector<uint8_t>{21}
    );
    CHECK(
       population.getAttribute<int8_t>("attr-Y", Selection({{0, 1}})) == std::vector<int8_t>{21}
    );
    CHECK(
       population.getAttribute<std::string>("attr-Z", Selection({{0, 2}})) == std::vector<std::string>{"aa", "bb"}
    );
    CHECK(
       population.getAttribute<std::string>("attr-Z", Selection({{0, 1}, {5, 6}})) == std::vector<std::string>{"aa", "ff"}
    );
    CHECK_THROWS_AS(
       population.getAttribute<double>("no-such-attribute", Selection({{0, 1}})),
       SonataError
    );

    // getAttribute with default value
    CHECK(
       population.getAttribute<double>("attr-X", Selection({{0, 1}, {5, 6}}), 42.0) == std::vector<double>{11.0, 16.0}
    );
    CHECK_THROWS_AS(
       population.getAttribute<double>("no-such-attribute", Selection({{0, 1}}), 42.0),
       SonataError
    );

    CHECK(population._attributeDataType("attr-X") == "double");
    CHECK(population._attributeDataType("attr-Y") == "int64_t");
    CHECK(population._attributeDataType("attr-Z") == "string");
    CHECK_THROWS_AS(
       population._attributeDataType("no-such-attribute"),
       SonataError
    );
}
