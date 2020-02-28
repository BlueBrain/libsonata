#include <catch2/catch.hpp>

#include <bbp/sonata/output.h>

using namespace bbp::sonata;

TEST_CASE("SpikeReader", "[base]") {
    const SpikeReader reader("./data/spikes.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "spikes1", "spikes2"});

    REQUIRE(reader["All"].get(Selection({{3,4}})) == std::vector<std::pair<uint64_t, double>>{{3UL, 0.3}, {3UL, 1.3}});

    REQUIRE(reader["spikes1"].getSorting() == SpikeReader::Population::Sorting::by_id);
}

TEST_CASE("ReportReader", "[base]") {
    const ReportReader reader("./data/somas.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "soma1", "soma2"});

    REQUIRE(reader["All"].getTimes() == std::make_tuple(0., 1., 0.1));

    REQUIRE(reader["All"].getTimeUnits() == "ms");

    REQUIRE(reader["All"].getDataUnits() == "mV");

    REQUIRE(reader["All"].getSorted());

    REQUIRE(reader["All"].get(Selection({{3, 5}}), 0.2, 0.5) ==
            std::vector<std::vector<float>>{{4.2f, 6.2f, 8.2f}, {4.3f, 6.3f, 8.3f}});
}
