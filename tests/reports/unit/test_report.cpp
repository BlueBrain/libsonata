#include <array>
#include <catch2/catch.hpp>
#include <memory>
#include <reports/library/element_report.h>
#include <reports/library/report.h>
#include <reports/library/soma_report.h>

using namespace bbp::sonata;

static const std::string population_name = "All";

SCENARIO("Test Report class", "[Report]") {
    uint32_t soma_id = 42;
    uint32_t element_id = 142;
    GIVEN("An instance of a soma report") {
        const double tstart = 0;
        const double tend = 10.0;
        const double dt = 0.3;
        std::shared_ptr<Report> soma_report =
            std::make_shared<SomaReport>("somaReport", tstart, tend, dt);

        THEN("Number of steps is 34 (33 + 1)") {
            REQUIRE(soma_report->get_num_steps() == 34);
        }
        WHEN("We add a node and a variable to a soma report") {
            soma_report->add_node(population_name, 1);
            double element_value = 10;
            std::shared_ptr<Node> node = soma_report->get_node(population_name, 1);
            node->add_element(&element_value, soma_id);
            THEN("Number of nodes and elements is 1") {
                REQUIRE(soma_report->get_num_nodes(population_name) == 1);
                REQUIRE(soma_report->get_total_elements(population_name) == 1);
                REQUIRE(soma_report->node_exists(population_name, 1) == true);
                REQUIRE(soma_report->node_exists(population_name, 2) == false);
                REQUIRE(soma_report->is_empty() == false);
            }
        }
        WHEN("We add 2 elements to a given node in a soma report") {
            soma_report->add_node(population_name, 1);
            double element_value = 42;
            soma_report->get_node(population_name, 1)->add_element(&element_value, soma_id);
            REQUIRE_THROWS(
                soma_report->get_node(population_name, 1)->add_element(&element_value, soma_id));
            THEN("Number of nodes and elements is still 1") {
                REQUIRE(soma_report->get_num_nodes(population_name) == 1);
                REQUIRE(soma_report->get_total_elements(population_name) == 1);
            }
        }
        WHEN("We add the same node twice in the same population") {
            soma_report->add_node(population_name, 1);
            REQUIRE_THROWS(soma_report->add_node(population_name, 1));
            THEN("Number of nodes is 1") {
                REQUIRE(soma_report->get_num_nodes(population_name) == 1);
            }
        }
    }

    GIVEN("An instance of a element report") {
        const double tstart = 0;
        const double tend = 1.0;
        const double dt = 0.1;
        std::shared_ptr<Report> element_report =
            std::make_shared<ElementReport>("elementReport", tstart, tend, dt);
        THEN("Number of steps is 10") {
            REQUIRE(element_report->get_num_steps() == 10);
        }
        WHEN("We add 2 nodes") {
            element_report->add_node(population_name, 1);
            element_report->add_node(population_name, 2);

            THEN("Number of nodes is 2") {
                REQUIRE(element_report->get_num_nodes(population_name) == 2);
            }
        }
        WHEN("We add the same node twice") {
            element_report->add_node(population_name, 1);
            REQUIRE_THROWS(element_report->add_node(population_name, 1));
            THEN("Number of nodes is 1") {
                REQUIRE(element_report->get_num_nodes(population_name) == 1);
            }
        }

        WHEN("We add 10 elements") {
            element_report->add_node(population_name, 1);
            element_report->add_node(population_name, 2);
            double element_value = 10;
            element_report->get_node(population_name, 1)->add_element(&element_value, element_id);

            std::array<double, 9> elements{1, 20, 300, 4000, 500, 60, 7, 0.8, 9};
            for (auto element_value : elements) {
                element_report->get_node(population_name, 2)
                    ->add_element(&element_value, element_id);
            }

            // Doesn't exists
            REQUIRE_THROWS(element_report->get_node(population_name, 3)
                               ->add_element(&element_value, element_id));
            THEN("Number of nodes is 2") {
                REQUIRE(element_report->get_num_nodes(population_name) == 2);
            }
            THEN("The number of elements is 6") {
                REQUIRE(element_report->get_total_elements(population_name) == 10);
            }
        }
    }
}
