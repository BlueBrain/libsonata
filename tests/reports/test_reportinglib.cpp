#include <memory>
#include <catch2/catch.hpp>
#include <bbp/reports/records.h>

std::vector<double> elements {3.45, 563.12, 23.4, 779.2, 42.1};

SCENARIO( "Test reportinglib API", "[reportinglib]" ) {
    const double tstart = 0;
    const double tend = 0.3;
    const double dt = 0.1;

    const int mapping_size = 1;
    int mapping[mapping_size] = {142};

    WHEN("We add a new report (element) and node") {
        const char* report_name = "myElementReport";
        records_add_report(report_name, 1, 1, 1, tstart, tend, dt, 0, "element", 0, nullptr);
        THEN( "Number of reports is 1" ) {
            REQUIRE( records_get_num_reports() == 1 );
        }
    }

    WHEN("We add a node to an existing element report") {
        const char* report_name = "myElementReport";
        records_add_report(report_name, 2, 2, 2, tstart, tend, dt, 0, "element", 0, nullptr);
        THEN( "Number of reports is still 1" ) {
            REQUIRE( records_get_num_reports() == 1 );
        }
    }

    WHEN("We add an existing node to an existing element report") {
        const char* report_name = "myElementReport";
        records_add_report(report_name, 2, 2, 2, tstart, tend, dt, 0, "element", 0, nullptr);
        THEN( "Number of reports is still 1" ) {
            REQUIRE( records_get_num_reports() == 1 );
        }
    }

    WHEN("We add a second report (soma), a node and a variable") {
        const char* report_name = "mySomaReport";
        records_add_report(report_name, 1, 1, 1, tstart, tend, dt, 0, "soma", 0, nullptr);
        double soma_value = 42;
        records_add_var_with_mapping(report_name, 1, &soma_value, mapping_size, mapping);
        THEN( "Number of reports is 2" ) {
            REQUIRE( records_get_num_reports() == 2 );
        }
    }

    WHEN("We add a second variable to an existing node in a soma report") {
        const char* report_name = "mySomaReport";
        double soma_value = 42;
        records_add_var_with_mapping(report_name, 1, &soma_value, mapping_size, mapping);
        THEN( "Number of reports is 2" ) {
            REQUIRE( records_get_num_reports() == 2 );
        }
    }

    WHEN("We add a report of a non existing type") {
        const char* report_name = "myWeirdReport";
        records_add_report(report_name, 44, 44, 44, tstart, tend, dt, 0, "weird", 0, nullptr);
        THEN( "Number of reports is still 2" ) {
            REQUIRE( records_get_num_reports() == 2 );
        }
    }

    WHEN("10 nodes with 5 elements each") {
        // Report Name
        const char* report_name = "elementReport";
        // 10 nodes
        std::vector<uint64_t> node_ids{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        for(int node_id: node_ids) {
            records_add_report(report_name, node_id, node_id, node_id, tstart, tend, dt, 0, "element", 0, nullptr);
            for(double& voltage: elements) {
                records_add_var_with_mapping(report_name, node_id, &voltage, mapping_size, mapping);
            }
        }
        THEN( "Number of reports is 3" ) {
            REQUIRE( records_get_num_reports() == 3 );
        }
    }

    WHEN("We record data") {
        const char *report_name = "elementReport";
        int nodeids[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        int num_steps = static_cast<int>((tend - tstart) / dt);
        if (std::fabs(num_steps * dt + tstart - tend) > std::numeric_limits<float>::epsilon()) {
            num_steps++;
        }
        records_setup_communicator();
        records_finish_and_share();

        double t = 0.0;
        for (int i = 0; i < num_steps; i++) {
            records_nrec(t, 10, nodeids, report_name);
            records_end_iteration(t);
            t+=dt;
        }
        records_flush(t);
        THEN("Number of reports is still 3") {
            REQUIRE(records_get_num_reports() == 3);
        }
    }
}
