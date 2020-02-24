#include <catch2/catch.hpp>

#include <bbp/sonata/output.h>

using namespace bbp::sonata;

TEST_CASE("SpikeReader", "[base]") {
    const SpikeReader reader("./data/spikes.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "spikes1", "spikes2"});

    REQUIRE(reader["All"].get(Selection({{3,4}})) == std::vector<std::pair<uint64_t, double>>{{3UL, 0.3}, {3UL, 1.3}});
}
