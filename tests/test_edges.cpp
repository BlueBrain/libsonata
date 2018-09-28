#include <bbp/sonata/edges.h>

#include "contrib/catch.hpp"

#include <iostream>
#include <string>
#include <vector>


using namespace bbp::sonata;


namespace std {

std::ostream& operator<<(std::ostream& oss, const EdgeSelection& selection)
{
    oss << "{ ";
    for (const auto& range: selection.ranges()) {
        oss << range.first << ":" << range.second << " ";
    }
    oss << "}";
    return oss;
}

}  // unnamed namespace


bool operator==(const EdgeSelection& lhs, const EdgeSelection& rhs)
{
    return lhs.ranges() == rhs.ranges();
}


TEST_CASE("EdgeStorage", "[edges]")
{
    // CSV not supported at the moment
    CHECK_THROWS_AS(
        EdgeStorage("./data/edges1.h5", "csv-file"),
        SonataError
    );

    const EdgeStorage edges("./data/edges1.h5");
    CHECK(
        edges.populationNames() == std::set<std::string>{"edges-AB", "edges-AC"}
    );

    CHECK(
        edges.openPopulation("edges-AB")
    );
    CHECK_THROWS_AS(
        edges.openPopulation("no-such-population"),
        SonataError
    );
    // multi-group populations not supported at the moment
    CHECK_THROWS_AS(
        edges.openPopulation("edges-AC"),
        SonataError
    );
}


TEST_CASE("EdgeSelection", "[edges]")
{
    SECTION("range check") {
        CHECK_THROWS_AS(
            EdgeSelection({{0, 2}, {3, 3}}),
            SonataError
        );
        CHECK_THROWS_AS(
            EdgeSelection({{5, 3}}),
            SonataError
        );
    }
    SECTION("fromValues") {
        const auto selection = EdgeSelection::fromValues({1, 3, 4, 1});
        CHECK(selection.ranges() == EdgeSelection::Ranges{{1, 2}, {3, 5}, {1, 2}});
    }
    SECTION("empty") {
        const auto selection = EdgeSelection({});
        CHECK(selection.ranges().empty());
        CHECK(selection.flatten().empty());
        CHECK(selection.flatSize() == 0);
        CHECK(selection.empty());
    }
    SECTION("basic") {
        const EdgeSelection::Ranges ranges{{3, 5}, {0, 3}};
        EdgeSelection selection(ranges);  // copying ranges

        CHECK(selection.ranges() == ranges);
        CHECK(selection.flatten() == std::vector<EdgeID>{3, 4, 0, 1, 2});
        CHECK(selection.flatSize() == 5);
        CHECK(!selection.empty());
    }
}


TEST_CASE("EdgePopulation", "[edges]")
{
    const EdgeStorage edges("./data/edges1.h5");

    const auto population = edges.openPopulation("edges-AB");

    CHECK(population->name() == "edges-AB");
    CHECK(population->sourcePopulation() == "nodes-A");
    CHECK(population->targetPopulation() == "nodes-B");

    REQUIRE(population->size() == 5);

    CHECK(
        population->sourceNodeIDs(EdgeSelection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 1, 2, 3}
    );
    CHECK(
        population->targetNodeIDs(EdgeSelection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 2, 1, 0}
    );

    REQUIRE(population->attributeNames() == std::set<std::string>{"attr-X", "attr-Y", "attr-Z"});

    CHECK(
       population->getAttribute<double>("attr-X", EdgeSelection({{0, 1}, {5, 6}})) == std::vector<double>{11.0, 16.0}
    );
    CHECK(
       population->getAttribute<float>("attr-X", EdgeSelection({{0, 1}})) == std::vector<float>{11.0f}
    );
    CHECK(
       population->getAttribute<uint64_t>("attr-Y", EdgeSelection({{0, 1}, {5, 6}})) == std::vector<uint64_t>{21, 26}
    );
    CHECK(
       population->getAttribute<int64_t>("attr-Y", EdgeSelection({{0, 1}})) == std::vector<int64_t>{21}
    );
    CHECK(
       population->getAttribute<uint32_t>("attr-Y", EdgeSelection({{0, 1}})) == std::vector<uint32_t>{21}
    );
    CHECK(
       population->getAttribute<int32_t>("attr-Y", EdgeSelection({{0, 1}})) == std::vector<int32_t>{21}
    );
    CHECK(
       population->getAttribute<uint8_t>("attr-Y", EdgeSelection({{0, 1}})) == std::vector<uint8_t>{21}
    );
    CHECK(
       population->getAttribute<int8_t>("attr-Y", EdgeSelection({{0, 1}})) == std::vector<int8_t>{21}
    );
    CHECK(
       population->getAttribute<std::string>("attr-Z", EdgeSelection({{0, 2}})) == std::vector<std::string>{"aa", "bb"}
    );
    CHECK(
       population->getAttribute<std::string>("attr-Z", EdgeSelection({{0, 1}, {5, 6}})) == std::vector<std::string>{"aa", "ff"}
    );
    CHECK_THROWS_AS(
       population->getAttribute<double>("no-such-attribute", EdgeSelection({{0, 1}})),
       SonataError
    );

    // getAttribute with default value
    CHECK(
       population->getAttribute<double>("attr-X", EdgeSelection({{0, 1}, {5, 6}}), 42.0) == std::vector<double>{11.0, 16.0}
    );
    CHECK_THROWS_AS(
       population->getAttribute<double>("no-such-attribute", EdgeSelection({{0, 1}}), 42.0),
       SonataError
    );

    CHECK(population->_attributeDataType("attr-X") == "double");
    CHECK(population->_attributeDataType("attr-Y") == "int64_t");
    CHECK(population->_attributeDataType("attr-Z") == "string");
    CHECK_THROWS_AS(
       population->_attributeDataType("no-such-attribute"),
       SonataError
    );

    CHECK(
        population->afferentEdges({}).empty()
    );
    CHECK(
        population->afferentEdges({3}).empty()
    );
    CHECK(
        population->afferentEdges({1}) == EdgeSelection({{0, 1}, {2, 4}})
    );
    CHECK(
        population->afferentEdges({1, 2}) == EdgeSelection({{0, 4}, {5, 6}})
    );

    CHECK(
        population->efferentEdges({}).empty()
    );
    CHECK(
        population->efferentEdges({0}).empty()
    );
    CHECK(
        population->efferentEdges({1}) == EdgeSelection({{0, 2}})
    );
    CHECK(
        population->efferentEdges({1, 3}) == EdgeSelection({{0, 2}, {4, 6}})
    );
    CHECK(
        population->efferentEdges({1, 2}) == EdgeSelection({{0, 4}})
    );

    CHECK(
        population->connectingEdges({}, {0, 1, 2, 3}).empty()
    );
    CHECK(
        population->connectingEdges({0, 1, 2, 3}, {}).empty()
    );
    CHECK(
        population->connectingEdges({0, 1}, {0, 3}).empty()
    );
    CHECK(
        population->connectingEdges({3}, {0}) == EdgeSelection({{4, 5}})
    );
    CHECK(
        population->connectingEdges({1, 2}, {1, 2}) == EdgeSelection({{0, 4}})
    );
    CHECK(
        population->connectingEdges({0, 1, 2, 3}, {2}) == EdgeSelection({{1, 2}, {5, 6}})
    );
    // duplicate node IDs are ignored; order of node IDs is not relevant
    CHECK(
        population->connectingEdges({2, 1, 2}, {2, 1, 2}) == EdgeSelection({{0, 4}})
    );
}
