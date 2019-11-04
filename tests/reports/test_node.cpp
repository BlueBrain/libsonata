#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>
#include <reports/data/node.hpp>
#include <memory>

double* square(double* elem)
{
    *elem *= *elem;
    return elem;
}

SCENARIO( "Test Node class", "[Node]" ) {

    GIVEN( "An instance of a Node" ) {
        Node node{1};
        WHEN("We add a element") {
            std::vector<double> elements = {10, 11, 12, 13, 14};
            size_t i = 0;
            for(auto& element: elements) {
                node.add_element(&element, i);
                ++i;
            }
            THEN("Number of elements is 5") {
                REQUIRE(node.get_num_elements() == 5);
            }
            THEN("The element_ids are") {
                std::vector<uint32_t> compare = {0, 1, 2, 3, 4};
                REQUIRE(node.get_element_ids() == compare);
            }
            THEN("fill_data will return something correct") {
                std::vector<double> result(5, -1.);
                node.fill_data(result.data());
                REQUIRE(result == elements);
            }
            THEN("refresh_pointers will be call on all elements") {
                node.refresh_pointers(&square);
                std::vector<double> compare{100, 121, 144, 169, 196};
                std::vector<double> result(5, -1);
                node.fill_data(result.data());
                REQUIRE(result == compare);
            }
        }
    }

}
