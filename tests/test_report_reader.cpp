#include <catch2/catch.hpp>

#include <bbp/sonata/report_reader.h>

using namespace bbp::sonata;

void testTimes(const std::vector<double>& vec, double start, double step, int size) {
    REQUIRE(size == vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        REQUIRE(std::abs(vec[i] - (start + step * i)) < std::numeric_limits<double>::epsilon());
    }
}

TEST_CASE("SpikeReader", "[base]") {
    const SpikeReader reader("./data/spikes.h5");

    REQUIRE(reader.getPopulationNames() ==
            std::vector<std::string>{"All", "empty", "spikes1", "spikes2"});

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

    REQUIRE(reader.openPopulation("All").get(Selection({{5, 6}}), 0.1, 0.1) ==
            std::vector<std::pair<uint64_t, double>>{{5, 0.1}});
    REQUIRE(reader.openPopulation("empty").get() == std::vector<std::pair<uint64_t, double>>{});
}

TEST_CASE("SomaReportReader limits", "[base]") {
    const SomaReportReader reader("./data/somas.h5");

    auto pop = reader.openPopulation("All");

    // ids out of range
    REQUIRE(pop.get(Selection({{100, 101}})).ids == DataFrame<NodeID>::DataType{});

    // Inverted id
    REQUIRE_THROWS(pop.get(Selection({{2, 1}})));

    // Negative ids
    REQUIRE_THROWS(pop.get(Selection({{-1, 1}})));

    // Times out of range
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), 100., 101.));

    // Inverted times
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), 0.2, 0.1));

    // Negatives times
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), -1., -2.));
}

TEST_CASE("SomaReportReader", "[base]") {
    const SomaReportReader reader("./data/somas.h5");

    REQUIRE(reader.getPopulationNames() == std::vector<std::string>{"All", "soma1", "soma2"});

    auto pop = reader.openPopulation("All");

    REQUIRE(pop.getTimes() == std::make_tuple(0., 1., 0.1));

    REQUIRE(pop.getTimeUnits() == "ms");

    REQUIRE(pop.getDataUnits() == "mV");

    REQUIRE(pop.getSorted());

    REQUIRE(pop.getNodeIds() == std::vector<NodeID>{1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                                    11, 12, 13, 14, 15, 16, 17, 18, 19, 20});

    auto data = pop.get(Selection({{3, 5}}), 0.2, 0.5);
    REQUIRE(data.ids == DataFrame<NodeID>::DataType{{3, 4}});
    testTimes(data.times, 0.2, 0.1, 4);
    REQUIRE(data.data == std::vector<float>{3.2f, 4.2f, 3.3f, 4.3f, 3.4f, 4.4f, 3.5f, 4.5f});
}

TEST_CASE("ElementReportReader limits", "[base]") {
    const ElementReportReader reader("./data/elements.h5");

    auto pop = reader.openPopulation("All");

    // ids out of range
    REQUIRE(pop.get(Selection({{100, 101}})).ids ==
            DataFrame<std::pair<NodeID, ElementID>>::DataType{});

    // Inverted id
    REQUIRE_THROWS(pop.get(Selection({{2, 1}})));

    // Negative ids
    REQUIRE_THROWS(pop.get(Selection({{-1, 1}})));

    // Times out of range
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), 100., 101.));

    // Inverted times
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), 0.2, 0.1));

    // Negatives times
    REQUIRE_THROWS(pop.get(Selection({{1, 2}}), -1., -2.));
}

TEST_CASE("ElementReportReader", "[base]") {
    const ElementReportReader reader("./data/elements.h5");

    REQUIRE(reader.getPopulationNames() ==
            std::vector<std::string>{"All", "element1", "element42"});

    auto pop = reader.openPopulation("All");

    REQUIRE(pop.getTimes() == std::make_tuple(0., 4., 0.2));

    REQUIRE(pop.getTimeUnits() == "ms");

    REQUIRE(pop.getDataUnits() == "mV");

    REQUIRE(pop.getSorted());

    REQUIRE(pop.getNodeIds() == std::vector<NodeID>{1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                                    11, 12, 13, 14, 15, 16, 17, 18, 19, 20});

    auto data = pop.get(Selection({{3, 5}}), 0.2, 0.5);
    REQUIRE(data.ids ==
            DataFrame<std::pair<NodeID, ElementID>>::DataType{
                {{3, 5}, {3, 5}, {3, 6}, {3, 6}, {3, 7}, {4, 7}, {4, 8}, {4, 8}, {4, 9}, {4, 9}}});
    testTimes(data.times, 0.2, 0.2, 2);
    REQUIRE(data.data == std::vector<float>{11.0f, 11.1f, 11.2f, 11.3f, 11.4f, 11.5f, 11.6f,
                                            11.7f, 11.8f, 11.9f, 21.0f, 21.1f, 21.2f, 21.3f,
                                            21.4f, 21.5f, 21.6f, 21.7f, 21.8f, 21.9f});

    // Select only one time
    REQUIRE(pop.get(Selection({{1, 2}}), 0.6, 0.6).data ==
            std::vector<float>{30.0f, 30.1f, 30.2f, 30.3f, 30.4f});
}
