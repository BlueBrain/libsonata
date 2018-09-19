#include <bbp/sonata/edges.h>

#include "contrib/catch.hpp"

#include <string>
#include <vector>


using namespace bbp::sonata;


TEST_CASE("EdgeStorage", "[edges]")
{
    const EdgeStorage edges("./data/edges1.h5");
    CHECK(
        edges.populationNames() == std::set<std::string>{"edges-AB"}
    );
    CHECK(
        edges.openPopulation("edges-AB")
    );
    CHECK_THROWS(
        edges.openPopulation("no-such-population")
    );

    CHECK_THROWS_AS(
        EdgeStorage("./data/edges1.h5", "csv-file"),
        SonataError
    );
}


TEST_CASE("EdgePopulation", "[edges]")
{
    const EdgeStorage edges("./data/edges1.h5");
    const auto population = edges.openPopulation("edges-AB");

    CHECK(population->name() == "edges-AB");
    CHECK(population->sourcePopulation() == "nodes-A");
    CHECK(population->targetPopulation() == "nodes-B");

    REQUIRE(population->size() == 5);
}
