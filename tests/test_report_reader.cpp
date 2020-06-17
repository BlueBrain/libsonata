#include <catch2/catch.hpp>

#include <bbp/sonata/report_reader.h>

using namespace bbp::sonata;

bool testTimes(std::vector<double> vec, double start, double step, int size) {
    REQUIRE(size == vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        if (std::abs(vec[i] - (start + step * i) > std::numeric_limits<double>::epsilon())) {
            return false;
        }
    }
    return true;
}

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

TEST_CASE("ReportReader limits", "[base]") {
    const SomaReportReader reader("./data/somas.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "soma1", "soma2"});

    auto pop = reader.openPopulation("All");

    // ids out of range
    REQUIRE(pop.get(Selection({{100, 101}})).ids == DataFrame<NodeID>::DataType());
    REQUIRE(testTimes(pop.get(Selection({{100, 101}})).times, 0.0, 0.1, 10) == true);
    REQUIRE(pop.get(Selection({{100, 101}})).data ==
            std::vector<std::vector<float>>({{}, {}, {}, {}, {}, {}, {}, {}, {}, {}}));

    // Inverted id
    REQUIRE_THROWS(pop.get(Selection({{2, 1}})));

    // Negative ids
    REQUIRE_THROWS(pop.get(Selection({{-1, 1}})));

    // Times out of range
    REQUIRE(pop.get(Selection({{1, 2}}), 100., 101.).ids == DataFrame<NodeID>::DataType());
    REQUIRE(testTimes(pop.get(Selection({{1, 2}}), 100., 101.).times, 0.0, 0.0, 0) == true);
    REQUIRE(pop.get(Selection({{1, 2}}), 100., 101.).data == std::vector<std::vector<float>>({}));

    // Negatives times
    REQUIRE(pop.get(Selection({{1, 2}}), -1., -2.).ids == DataFrame<NodeID>::DataType({1}));
    REQUIRE(testTimes(pop.get(Selection({{1, 2}}), -1., -2.).times, 0.0, 0.1, 10) == true);
    REQUIRE(pop.get(Selection({{1, 2}}), -1., -2.).data ==
            std::vector<std::vector<float>>(
                {{1.0}, {1.1}, {1.2}, {1.3}, {1.4}, {1.5}, {1.6}, {1.7}, {1.8}, {1.9}}));
}

TEST_CASE("ReportReader", "[base]") {
    const SomaReportReader reader("./data/somas.h5");

    REQUIRE(reader.getPopulationsNames() == std::vector<std::string>{"All", "soma1", "soma2"});

    auto pop = reader.openPopulation("All");

    REQUIRE(pop.getTimes() == std::make_tuple(0., 1., 0.1));

    REQUIRE(pop.getTimeUnits() == "ms");

    REQUIRE(pop.getDataUnits() == "mV");

    REQUIRE(pop.getSorted());

    auto data = pop.get(Selection({{3, 5}}), 0.2, 0.5);
    REQUIRE(data.ids == DataFrame<NodeID>::DataType{{3, 4}});
    REQUIRE(testTimes(data.times, 0.2, 0.1, 4) == true);
    REQUIRE(data.data == std::vector<std::vector<float>>{
                             {{3.2f, 4.2f}, {3.3f, 4.3f}, {3.4f, 4.4f}, {3.5f, 4.5f}}});
}
