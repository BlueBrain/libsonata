#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>
#include <reports/data/node.hpp>
#include <memory>

SCENARIO( "Test Node class", "[Node]" ) {

    GIVEN( "An instance of a Node" ) {
        Node node(3,3);
        WHEN("We add a element") {
            std::vector<double> elements = {10, 11, 12, 13, 14};
            int i = 0;
            for(auto& element: elements) {
                node.add_element(&element, i);
                ++i;
            }
            THEN("Number of elements is 5") {
                REQUIRE(node.get_num_elements() == 5);
            }
            THEN("The element_ids are") {
                //std::vector<uint32_t> compare = {3000, 3001, 3002, 3003, 3004};
                std::vector<uint32_t> compare = {0, 1, 2, 3, 4};
                REQUIRE(node.get_element_ids() == compare);
            }
        }
    }

}
