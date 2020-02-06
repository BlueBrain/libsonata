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

    SECTION("intersection") {
        const auto empty = Selection({});
        CHECK(empty == (empty & empty));

        // clang-format off
        //              1         2
        //    01234567890123456789012345
        // a = xx   xxxxx   xxxxxxxxxx x
        // b =  xxxxx  xxxxx  xxxxxxxx x
        //      x   x  xx     xxxxxxxx x   <- expected
        // clang-format on
        const auto a = Selection({{24, 25}, {13, 23}, {5, 10}, {0, 2}});
        CHECK(empty == (a & empty));
        CHECK(empty == (empty & a));

        const auto b = Selection({{1, 6}, {8, 13}, {15, 23}, {24, 25}});
        CHECK(b == (b & b));  // Note: b is already sorted

        const auto expected = Selection({{1, 2}, {5, 6}, {8, 10}, {15, 23}, {24, 25}});
        CHECK(expected == (a & b));
        CHECK(expected == (b & a));

        const auto odd = Selection::fromValues({1, 3, 5, 7, 9});
        const auto even = Selection::fromValues({0, 2, 4, 6, 8});
        CHECK(empty == (odd & even));
        CHECK(empty == (even & odd));
    }

    SECTION("union") {
        const auto empty = Selection({});
        CHECK(empty == (empty | empty));

        // clang-format off
        //              1         2
        //    01234567890123456789012345
        // a = xx   xxxxx   xxxxxxxxxx x
        // b =  xxxxx  xxxxx  xxxxxxxx x
        //     xxxxxxxxxxxxxxxxxxxxxxx x
        // clang-format on
        const auto a = Selection({{24, 25}, {13, 23}, {5, 10}, {0, 2}});
        const auto b = Selection({{1, 6}, {8, 13}, {15, 23}, {24, 25}});
        CHECK(b == (b | empty));  // need to use b since it's sorted
        CHECK(b == (empty | b));

        const auto expected = Selection({{0, 23}, {24, 25}});
        CHECK(expected == (a | b));
        CHECK(expected == (b | a));

        const auto odd = Selection::fromValues({1, 3, 5, 7, 9});
        const auto even = Selection::fromValues({0, 2, 4, 6, 8});
        CHECK(Selection({{0, 10}}) == (odd | even));
        CHECK(Selection({{0, 10}}) == (even | odd));
    }

    /*  need a way to test un-exported stuff
    SECTION("_sortAndMerge") {
        const auto empty = Selection::Ranges({});
        CHECK(empty == _sortAndMerge(empty));

        const auto unsorted_no_overlap = Selection::Ranges({{10, 20}, {0, 4}});
        CHECK(Selection::Ranges({{0, 4}, {10, 20}}) == _sortAndMerge(unsorted_no_overlap));

        {
        const auto sorted_with_overlap = Selection::Ranges({{0, 1}, {1, 2}});
        CHECK(Selection::Ranges({{0, 2}}) == _sortAndMerge(sorted_with_overlap));
        }

        {
            const auto duplicates = Selection::Ranges({{1, 2}, {0, 1}, {1, 2}, {0, 1},
                {1, 2}, {0, 1}, {1, 2}});
            CHECK(Selection::Ranges({{0, 2}}) == _sortAndMerge(duplicates));
        }
    }
    */
}
