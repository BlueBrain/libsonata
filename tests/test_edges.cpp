#include "contrib/catch.hpp"

#include <bbp/sonata/edges.h>

#include <iostream>
#include <string>
#include <vector>


using namespace bbp::sonata;


namespace std {

std::ostream& operator<<(std::ostream& oss, const Selection& selection)
{
    oss << "{ ";
    for (const auto& range: selection.ranges()) {
        oss << range.first << ":" << range.second << " ";
    }
    oss << "}";
    return oss;
}

}


namespace bbp {
namespace sonata {

bool operator==(const Selection& lhs, const Selection& rhs)
{
    return lhs.ranges() == rhs.ranges();
}

}
}


TEST_CASE("EdgePopulation", "[edges]")
{
    const EdgePopulation population("./data/edges1.h5", "", "edges-AB");

    CHECK(population.source() == "nodes-A");
    CHECK(population.target() == "nodes-B");

    REQUIRE(population.size() == 6);

    CHECK(
        population.sourceNodeIDs(Selection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 1, 2, 3}
    );
    CHECK(
        population.targetNodeIDs(Selection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 2, 1, 0}
    );

    CHECK(
        population.afferentEdges({}).empty()
    );
    CHECK(
        population.afferentEdges({3}).empty()
    );
    CHECK(
        population.afferentEdges({1}) == Selection({{0, 1}, {2, 4}})
    );
    CHECK(
        population.afferentEdges({1, 2}) == Selection({{0, 4}, {5, 6}})
    );
    CHECK(
        population.afferentEdges({999}).empty()
    );

    CHECK(
        population.efferentEdges({}).empty()
    );
    CHECK(
        population.efferentEdges({0}).empty()
    );
    CHECK(
        population.efferentEdges({1}) == Selection({{0, 2}})
    );
    CHECK(
        population.efferentEdges({1, 3}) == Selection({{0, 2}, {4, 6}})
    );
    CHECK(
        population.efferentEdges({1, 2}) == Selection({{0, 4}})
    );
    CHECK(
        population.efferentEdges({999}).empty()
    );

    CHECK(
        population.connectingEdges({}, {0, 1, 2, 3}).empty()
    );
    CHECK(
        population.connectingEdges({0, 1, 2, 3}, {}).empty()
    );
    CHECK(
        population.connectingEdges({0, 1}, {0, 3}).empty()
    );
    CHECK(
        population.connectingEdges({3}, {0}) == Selection({{4, 5}})
    );
    CHECK(
        population.connectingEdges({1, 2}, {1, 2}) == Selection({{0, 4}})
    );
    CHECK(
        population.connectingEdges({0, 1, 2, 3}, {2}) == Selection({{1, 2}, {5, 6}})
    );
    CHECK(
        population.connectingEdges({999}, {999}).empty()
    );
    // duplicate node IDs are ignored; order of node IDs is not relevant
    CHECK(
        population.connectingEdges({2, 1, 2}, {2, 1, 2}) == Selection({{0, 4}})
    );
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