#include <catch2/catch.hpp>

#include <bbp/sonata/population.h>


using namespace bbp::sonata;


TEST_CASE("Selection", "[base]") {
    SECTION("range check") {
        CHECK_THROWS_AS(Selection({{0, 2}, {3, 3}}), SonataError);
        CHECK_THROWS_AS(Selection({{5, 3}}), SonataError);
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
    SECTION("comparison") {
        const auto empty = Selection({});
        const auto range_selection = Selection({{0, 2}, {3, 4}});
        const auto values_selection = Selection::fromValues({1, 3, 4, 1});
        const auto values_selection1 = Selection::fromValues({1, 3, 4, 1});

        CHECK(empty == empty);
        CHECK(empty != range_selection);
        CHECK(empty != values_selection);

        CHECK(range_selection == range_selection);
        CHECK(range_selection != values_selection);

        CHECK(values_selection == values_selection);
        CHECK(values_selection == values_selection1);
    }
}
