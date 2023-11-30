#include <catch2/catch.hpp>

#include <bbp/sonata/edges.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


using namespace bbp::sonata;


namespace std {

std::ostream& operator<<(std::ostream& oss, const Selection& selection) {
    oss << "{ ";
    for (const auto& range : selection.ranges()) {
        oss << std::get<0>(range) << ":" << std::get<1>(range) << " ";
    }
    oss << "}";
    return oss;
}

}  // namespace std


TEST_CASE("EdgePopulation", "[edges]") {
    // E-mapping-good              Dataset {6}
    // Data:
    //      2, 1, 2, 0, 2, 2
    //
    // @library/E-mapping-good     Dataset {3}
    // Data:
    //      "A", "B", "C"

    const EdgePopulation population("./data/edges1.h5", "", "edges-AB");

    CHECK(population.source() == "nodes-A");
    CHECK(population.target() == "nodes-B");

    REQUIRE(population.size() == 6);

    CHECK(population.sourceNodeIDs(Selection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 1, 2, 3});
    CHECK(population.targetNodeIDs(Selection({{0, 3}, {4, 5}})) == std::vector<NodeID>{1, 2, 1, 0});

    CHECK(population.afferentEdges({}).empty());
    CHECK(population.afferentEdges({3}).empty());
    CHECK(population.afferentEdges({1}) == Selection({{0, 1}, {2, 4}}));
    CHECK(population.afferentEdges({1, 2}) == Selection({{0, 4}, {5, 6}}));
    CHECK(population.afferentEdges({999}).empty());

    CHECK(population.efferentEdges({}).empty());
    CHECK(population.efferentEdges({0}).empty());
    CHECK(population.efferentEdges({1}) == Selection({{0, 2}}));
    CHECK(population.efferentEdges({1, 3}) == Selection({{0, 2}, {4, 6}}));
    CHECK(population.efferentEdges({1, 2}) == Selection({{0, 4}}));
    CHECK(population.efferentEdges({999}).empty());

    CHECK(population.connectingEdges({}, {0, 1, 2, 3}).empty());
    CHECK(population.connectingEdges({0, 1, 2, 3}, {}).empty());
    CHECK(population.connectingEdges({0, 1}, {0, 3}).empty());
    CHECK(population.connectingEdges({3}, {0}) == Selection({{4, 5}}));
    CHECK(population.connectingEdges({1, 2}, {1, 2}) == Selection({{0, 4}}));
    CHECK(population.connectingEdges({0, 1, 2, 3}, {2}) == Selection({{1, 2}, {5, 6}}));
    CHECK(population.connectingEdges({999}, {999}).empty());
    // duplicate node IDs are ignored; order of node IDs is not relevant
    CHECK(population.connectingEdges({2, 1, 2}, {2, 1, 2}) == Selection({{0, 4}}));

    // Canonical.
    CHECK(population.getAttribute<size_t>("E-mapping-good", Selection({{0, 1}, {2, 4}})) ==
          std::vector<size_t>{2, 2, 0});

    // Non-overlapping but not sorted.
    CHECK(population.getAttribute<size_t>("E-mapping-good", Selection({{2, 4}, {0, 1}})) ==
          std::vector<size_t>{2, 0, 2});

    // Sorted, but overlapping.
    CHECK(population.getAttribute<size_t>("E-mapping-good", Selection({{0, 2}, {1, 4}})) ==
          std::vector<size_t>{2, 1, 1, 2, 0});

    // Overlapping and not sorted.
    CHECK(population.getAttribute<size_t>("E-mapping-good", Selection({{1, 4}, {0, 2}})) ==
          std::vector<size_t>{1, 2, 0, 2, 1});

    CHECK(population.getAttribute<std::string>("E-mapping-good", Selection({{0, 1}})) ==
          std::vector<std::string>{"C"});
}


TEST_CASE("EdgePopulationSelectAll", "[base]") {
    const EdgePopulation population("./data/edges1.h5", "", "edges-AB");
    CHECK(population.selectAll().flatSize() == 6);
}


namespace {

// TODO: remove after switching to C++17
void copyFile(const std::string& srcFilePath, const std::string& dstFilePath) {
    std::ifstream src(srcFilePath, std::ios::binary);
    std::ofstream dst(dstFilePath, std::ios::binary);
    dst << src.rdbuf();
}

}  // unnamed namespace


TEST_CASE("EdgePopulation::writeIndices", "[edges]") {
    const std::string srcFilePath = "./data/edges-no-index.h5";
    const std::string dstFilePath = "./data/edges-no-index.h5.tmp";
    {
        const EdgePopulation population(srcFilePath, "", "edges-AB");

        // no index datasets yet
        CHECK_THROWS_AS(population.afferentEdges({1, 2}), SonataError);
        CHECK_THROWS_AS(population.efferentEdges({1, 2}), SonataError);
    }


    copyFile(srcFilePath, dstFilePath);

    try {
        EdgePopulation::writeIndices(dstFilePath, "edges-AB", 4, 4);
        {
            const EdgePopulation population(dstFilePath, "", "edges-AB");
            CHECK(population.afferentEdges({1, 2}) == Selection({{0, 4}, {5, 6}}));
            CHECK(population.efferentEdges({1, 2}) == Selection({{0, 4}}));
        }

        CHECK_THROWS_AS(
            EdgePopulation::writeIndices(dstFilePath, "edges-AB", 4, 4, /* overwrite */ false),
            SonataError);

        // Not implemented yet
        CHECK_THROWS_AS(
            EdgePopulation::writeIndices(dstFilePath, "edges-AB", 4, 4, /* overwrite */ true),
            SonataError);
    } catch (...) {
        try {
            std::remove(dstFilePath.c_str());
        } catch (...) {
        }
        throw;
    }

    std::remove(dstFilePath.c_str());
}


TEST_CASE("EdgeStorage", "[edges]") {
    // CSV not supported at the moment
    CHECK_THROWS_AS(EdgeStorage("./data/edges1.h5", "csv-file"), SonataError);

    const EdgeStorage edges("./data/edges1.h5");
    CHECK(edges.populationNames() == std::set<std::string>{"edges-AB", "edges-AC"});

    CHECK(edges.openPopulation("edges-AB"));
    CHECK_THROWS_AS(edges.openPopulation("no-such-population"), SonataError);
    // multi-group populations not supported at the moment
    CHECK_THROWS_AS(edges.openPopulation("edges-AC"), SonataError);
}
