#include <catch2/catch.hpp>
#include <reports/library/report.hpp>
#include <memory>

SCENARIO( "Test Report class", "[Report]" ) {
    const double tstart = 0;
    const double tend = 1.0;
    const double dt = 0.1;
    GIVEN( "An instance of a soma report" ) {
        std::shared_ptr<Report> soma_report = Report::create_report("somaReport", tstart, tend, dt, Report::Kind::SOMA);
        WHEN("We add a node and a variable to a soma report") {
            soma_report->add_node(1,1,1);
            double compartment_value = 10;
            soma_report->add_variable(1, &compartment_value);
            THEN("Number of nodes and compartments is 1") {
                REQUIRE(soma_report->get_num_nodes() == 1);
                REQUIRE(soma_report->get_total_compartments() == 1);
            }
        }
        WHEN("We add 2 compartments to a given node in a soma report") {
            soma_report->add_node(1,1,1);
            double compartment_value = 42;
            soma_report->add_variable(1, &compartment_value);
            soma_report->add_variable(1, &compartment_value);
            THEN("Number of nodes and compartments is still 1") {
                REQUIRE(soma_report->get_num_nodes() == 1);
                REQUIRE(soma_report->get_total_compartments() == 1);
            }
        }
    }

    GIVEN( "An instance of a compartment report" ) {
        std::shared_ptr<Report> compartment_report = Report::create_report("compartmentReport", tstart, tend, dt, Report::Kind::COMPARTMENT);
        WHEN("We add 2 nodes") {
            compartment_report->add_node(1,1,1);
            compartment_report->add_node(2,2,2);

            THEN("Number of nodes is 2") {
                REQUIRE(compartment_report->get_num_nodes() == 2);
            }
        }

        WHEN("We add 10 compartments") {
            compartment_report->add_node(1,1,1);
            compartment_report->add_node(2,2,2);
            double compartment_value = 10;
            compartment_report->add_variable(1, &compartment_value);

            std::vector<double> voltages = { 1, 20, 300, 4000, 500, 60, 7, 0.8, 9 };
            for(auto& voltage: voltages) {
                compartment_report->add_variable(2, &voltage);
            }

            // Doesn't exists
            compartment_report->add_variable(3, &compartment_value);
            THEN("Number of nodes is 2") {
                REQUIRE(compartment_report->get_num_nodes() == 2);
            }
            THEN("The number of compartments is 6") {
                REQUIRE(compartment_report->get_total_compartments() == 10);
            }
        }
    }

    GIVEN( "An instance of a spike report" ) {
        std::shared_ptr<Report> spike_report = Report::create_report("spikeReport", tstart, tend, dt, Report::Kind::SPIKE);
        WHEN("We try to add 2 nodes with same id to a spike report") {
            spike_report->add_node(1,1,1);
            spike_report->add_node(1,2,2);
            THEN("Number of nodes is still 1 (ignoring second insert)") {
                REQUIRE(spike_report->get_num_nodes() == 1);
            }
        }
    }
}
