#include "catch.hpp"
#include <reports/library/cell.hpp>
#include <memory>

SCENARIO( "Test Cell class", "[Cell]" ) {

    GIVEN( "An instance of a Cell" ) {
        Cell cell(3,3);
        WHEN("We add a compartment") {
            std::vector<double> compartments = {10, 11, 12, 13, 14};
            for(auto& compartment: compartments) {
                cell.add_compartment(&compartment);
            }
            THEN("Number of compartments is 5") {
                REQUIRE(cell.get_num_compartments() == 5);
            }
            THEN("The compartment_ids are") {
                std::vector<uint32_t> compare = {3000, 3001, 3002, 3003, 3004};
                REQUIRE(cell.get_compartment_ids() == compare);
            }
        }

        WHEN("We add 2 spikes") {
            double spike1 = 20;
            double spike2 = 30;
            cell.add_spike(&spike1);
            cell.add_spike(&spike2);
            THEN("Number of spikes is 2") {
                REQUIRE(cell.get_num_spikes() == 2);
            }
        }
    }

}
