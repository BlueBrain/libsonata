#include <catch2/catch.hpp>

#include <bbp/sonata/reports.h>

static const char* report_name = "myElementReport";

SCENARIO("Test SonataReport API", "[sonatareport]") {
    const double tstart = 0;
    const double tend = 0.3;
    const double dt = 0.1;

    uint32_t element_id = 142;

    WHEN("We add a new report (element) and node") {
        sonata_create_report(report_name, tstart, tend, dt, "compartment");
        sonata_add_node(report_name, 1);
        THEN("Number of reports is 1") {
            REQUIRE(sonata_get_num_reports() == 1);
        }
    }

    WHEN("We add a node to an existing element report") {
        sonata_create_report(report_name, tstart, tend, dt, "compartment");
        sonata_add_node(report_name, 2);
        THEN("Number of reports is still 1") {
            REQUIRE(sonata_get_num_reports() == 1);
        }
    }

    WHEN("We add an existing node to an existing element report") {
        sonata_create_report(report_name, tstart, tend, dt, "compartment");
        sonata_add_node(report_name, 2);
        THEN("Number of reports is still 1") {
            REQUIRE(sonata_get_num_reports() == 1);
        }
    }

    WHEN("We add a second report (soma), a node and a variable") {
        sonata_create_report(report_name, tstart, tend, dt, "soma");
        sonata_add_node(report_name, 1);
        double soma_value = 42;
        sonata_add_element(report_name, 1, element_id, &soma_value);
        THEN("Number of reports is 2") {
            REQUIRE(sonata_get_num_reports() == 2);
        }
    }

    WHEN("We add a second variable to an existing node in a soma report") {
        double soma_value = 24;
        sonata_add_element(report_name, 1, element_id, &soma_value);
        THEN("Number of reports is 2") {
            REQUIRE(sonata_get_num_reports() == 2);
        }
    }

    WHEN("We add a report of a non existing type") {
        sonata_create_report(report_name, tstart, tend, dt, "weird");
        THEN("Number of reports is still 2") {
            REQUIRE(sonata_get_num_reports() == 2);
        }
    }

    WHEN("10 nodes with 5 elements each") {
        // 10 nodes
        const std::vector<uint64_t> node_ids{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<double> elements{3.45, 563.12, 23.4, 779.2, 42.1};

        sonata_create_report(report_name, tstart, tend, dt, "compartment");
        for (int node_id : node_ids) {
            sonata_add_node(report_name, node_id);
            for (auto& voltage : elements) {
                sonata_add_element(report_name, node_id, element_id, &voltage);
            }
        }
        THEN("Number of reports is 3") {
            REQUIRE(sonata_get_num_reports() == 3);
        }
    }

    WHEN("We record data") {
        const int nodeids[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto num_steps = static_cast<int>((tend - tstart) / dt);
        if (std::fabs(num_steps * dt + tstart - tend) > std::numeric_limits<float>::epsilon()) {
            num_steps++;
        }
        sonata_set_atomic_step(dt);
        sonata_setup_communicators();
        sonata_prepare_datasets();

        double t = 0.0;
        for (int i = 0; i < num_steps; i++) {
            // sonata_record_node_data(i, 10, nodeids, report_name);
            sonata_record_data(i);
            sonata_end_iteration(t);
            t += dt;
        }
        sonata_flush(t);
        THEN("Number of reports is still 3") {
            REQUIRE(sonata_get_num_reports() == 3);
        }
    }
}
