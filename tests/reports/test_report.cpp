#include "catch.hpp"
#include <reports/library/report.hpp>
#include <memory>

SCENARIO( "Test Report class", "[Report]" ) {
    const double tstart = 0;
    const double tend = 1.0;
    const double dt = 0.1;
    GIVEN( "An instance of a soma report" ) {
        std::shared_ptr<Report> somaReport = Report::createReport("somaReport", tstart, tend, dt, SOMA);
        WHEN("We add a cell and a variable to a soma report") {
            somaReport->add_cell(1,1,1);
            double variableCompartment = 10;
            somaReport->add_variable(1, &variableCompartment);
            THEN("Number of cells and compartments is 1") {
                REQUIRE(somaReport->get_num_cells() == 1);
                REQUIRE(somaReport->get_total_compartments() == 1);
            }
        }
        WHEN("We add 2 compartments to a given cell in a soma report") {
            somaReport->add_cell(1,1,1);
            double variableCompartment = 42;
            somaReport->add_variable(1, &variableCompartment);
            somaReport->add_variable(1, &variableCompartment);
            THEN("Number of cells and compartments is still 1") {
                REQUIRE(somaReport->get_num_cells() == 1);
                REQUIRE(somaReport->get_total_compartments() == 1);
            }
        }
    }

    GIVEN( "An instance of a compartment report" ) {
        std::shared_ptr<Report> compartmentReport = Report::createReport("compartmentReport", tstart, tend, dt, COMPARTMENT);
        WHEN("We add 2 cells") {
            compartmentReport->add_cell(1,1,1);
            compartmentReport->add_cell(2,2,2);

            THEN("Number of cells is 2") {
                REQUIRE(compartmentReport->get_num_cells() == 2);
            }
        }

        WHEN("We add 10 compartments") {
            compartmentReport->add_cell(1,1,1);
            compartmentReport->add_cell(2,2,2);
            double variableCompartment = 10;
            compartmentReport->add_variable(1, &variableCompartment);

            std::vector<double> voltages = { 1, 20, 300, 4000, 500, 60, 7, 0.8, 9 };
            for(auto& voltage: voltages) {
                compartmentReport->add_variable(2, &voltage);
            }

            // Doesn't exists
            compartmentReport->add_variable(3, &variableCompartment);
            THEN("Number of cells is 2") {
                REQUIRE(compartmentReport->get_num_cells() == 2);
            }
            THEN("The number of compartments is 6") {
                REQUIRE(compartmentReport->get_total_compartments() == 10);
            }
        }
    }

    GIVEN( "An instance of a spike report" ) {
        std::shared_ptr<Report> spikeReport = Report::createReport("spikeReport", tstart, tend, dt, SPIKE);
        WHEN("We try to add 2 cells with same id to a spike report") {
            spikeReport->add_cell(1,1,1);
            spikeReport->add_cell(1,2,2);
            THEN("Number of cells is still 1 (ignoring second insert)") {
                REQUIRE(spikeReport->get_num_cells() == 1);
            }
        }
    }
}
