#include <catch2/catch.hpp>
#include <memory>
#include <data/node.h>
#include <data/soma_node.h>
#include <spdlog/spdlog.h>

using namespace bbp::sonata;

double* square(double* elem) {
    *elem *= *elem;
    return elem;
}

SCENARIO("Test Node class", "[Node]") {
    GIVEN("An instance of a Node") {
        Node node{1};
        {  // Initial conditions
            REQUIRE(node.get_node_id() == 1);
            REQUIRE(node.get_num_elements() == 0);
            REQUIRE(node.get_element_ids().empty());

            std::vector<float> res;
            node.fill_data(res.begin());
            REQUIRE(res.empty());

            REQUIRE_NOTHROW(node.refresh_pointers(&square));
        }

        WHEN("We add a element") {
            std::vector<double> elements = {10, 11, 12, 13, 14};
            size_t i = 0;
            for (auto& element : elements) {
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
                std::vector<float> result(5, -1.);
                node.fill_data(result.begin());
                REQUIRE(result == std::vector<float>(elements.begin(), elements.end()));
            }
            THEN("refresh_pointers will be call on all elements") {
                node.refresh_pointers(&square);
                std::vector<float> compare{100, 121, 144, 169, 196};
                std::vector<float> result(5, -1);
                node.fill_data(result.begin());
                REQUIRE(result == compare);
            }
        }
    }

    GIVEN("An instance of a soma node") {
        SomaNode node{1};
        WHEN("We add one element") {
            double elem1 = 1;
            node.add_element(&elem1, 1);

            THEN("Number of elements is 1") {
                REQUIRE(node.get_num_elements() == 1);
            }

            THEN("Adding an other element throw") {
                REQUIRE_THROWS(node.add_element(&elem1, 2));
                REQUIRE(node.get_num_elements() == 1);
            }
        }
    }
}
