#include <memory>
#include <catch2/catch.hpp>
#include <bbp/reports/records.h>

std::vector<double> compartments {3.45, 563.12, 23.4, 779.2, 42.1};

SCENARIO( "Test reportinglib API", "[reportinglib]" ) {
    const double tstart = 0;
    const double tend = 0.3;
    const double dt = 0.1;

    WHEN("We add a new report and node") {
        const char* report_name = "myReport";
        records_add_report(report_name, 1, 1, 1, tstart, tend, dt, "compartment");
        THEN( "Number of reports is 1" ) {
            REQUIRE( records_get_num_reports() == 1 );
        }
    }

    WHEN("We add a second report and node") {
        const char* report_name = "myReport2";
        records_add_report(report_name, 2, 2, 2, tstart, tend, dt, "soma");
        THEN( "Number of reports is 2" ) {
            REQUIRE( records_get_num_reports() == 2 );
        }
    }

    WHEN("We add a node to an existing report") {
        const char* report_name = "myReport";
        records_add_report(report_name, 3, 3, 3, tstart, tend, dt, "compartment");
        THEN( "Number of reports is still 2" ) {
            REQUIRE( records_get_num_reports() == 2 );
        }
    }

    WHEN("10 nodes with 5 compartments each") {
        // Report Name
        const char* report_name = "compartmentReport";
        // 10 nodes
        std::vector<uint64_t> node_ids{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        for(int node_id: node_ids) {
            records_add_report(report_name, node_id, node_id, node_id, tstart, tend, dt, "compartment");
            for(double& voltage: compartments) {
                records_add_var_with_mapping(report_name, node_id, &voltage);
            }
        }

        THEN( "Number of reports is 3" ) {
            REQUIRE( records_get_num_reports() == 3 );
        }
    }

    WHEN("We record data") {
        const char *report_name = "compartmentReport";
        uint64_t nodeids[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

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


