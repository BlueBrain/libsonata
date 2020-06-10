#include <catch2/catch.hpp>

#include <bbp/sonata/report_reader.h>

using namespace bbp::sonata;

TEST_CASE("SpikeReader", "[base]") {
    const SpikeReader reader("./data/spikes.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "spikes1", "spikes2"});

    REQUIRE(reader.openPopulation("All").get(Selection({{3, 4}})) ==
            std::vector<std::pair<uint64_t, double>>{{3UL, 0.3}, {3UL, 1.3}});
    REQUIRE(reader.openPopulation("spikes1").get(Selection({{3, 4}})) ==
            std::vector<std::pair<uint64_t, double>>{{3UL, 0.3}, {3UL, 1.3}});
    REQUIRE(reader.openPopulation("spikes2").get(Selection({{3, 4}})) ==
            std::vector<std::pair<uint64_t, double>>{{3UL, 0.3}, {3UL, 1.3}});

    REQUIRE(reader.openPopulation("All").getSorting() == SpikeReader::Population::Sorting::by_time);
    REQUIRE(reader.openPopulation("spikes1").getSorting() ==
            SpikeReader::Population::Sorting::by_id);
    REQUIRE(reader.openPopulation("spikes2").getSorting() ==
            SpikeReader::Population::Sorting::none);
}

TEST_CASE("ReportReader", "[base]") {
    const SomaReportReader reader("./data/somas.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "soma1", "soma2"});

    REQUIRE(reader.openPopulation("All").getTimes() == std::make_tuple(0., 1., 0.1));

    REQUIRE(reader.openPopulation("All").getTimeUnits() == "ms");

    REQUIRE(reader.openPopulation("All").getDataUnits() == "mV");

    REQUIRE(reader.openPopulation("All").getSorted());

    REQUIRE(reader.openPopulation("All").get(Selection({{3, 5}}), 0.2, 0.5).ids ==
            DataFrame<NodeID>::DataType{{3, 4}});

    REQUIRE(
        reader.openPopulation("All").get(Selection({{3, 5}}), 0.2, 0.5).data ==
        std::vector<std::vector<float>>{{{3.2f, 4.2f}, {3.3f, 4.3f}, {3.4f, 4.4f}, {3.5f, 4.5f}}});
}
