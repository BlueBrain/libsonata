#include <catch2/catch.hpp>
#include <reports/library/node.hpp>
#include <memory>

SCENARIO( "Test Node class", "[Node]" ) {

    GIVEN( "An instance of a Node" ) {
        Node node(3,3);
        WHEN("We add a compartment") {
            std::vector<double> compartments = {10, 11, 12, 13, 14};
            for(auto& compartment: compartments) {
                node.add_compartment(&compartment);
            }
            THEN("Number of compartments is 5") {
                REQUIRE(node.get_num_compartments() == 5);
            }
            THEN("The compartment_ids are") {
                std::vector<uint32_t> compare = {3000, 3001, 3002, 3003, 3004};
                REQUIRE(node.get_compartment_ids() == compare);
            }
        }

        WHEN("We add 2 spikes") {
            double spike1 = 20;
            double spike2 = 30;
            node.add_spike(&spike1);
            node.add_spike(&spike2);
            THEN("Number of spikes is 2") {
                REQUIRE(node.get_num_spikes() == 2);
            }
        }
    }

}
