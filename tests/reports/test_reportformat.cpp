#include <memory>
#include <iostream>
#include <catch2/catch.hpp>
#include <reports/library/report_format.hpp>
#include <reports/library/sonata_format.hpp>

SCENARIO( "Test ReportFormat class", "[ReportFormat][IOWriter]" ) {
    GIVEN( "A cell map structure" ) {
        using cells_t = std::map<int, Cell>;
        Cell cell(1,1);
        double compartment = 10;
        double compartment2 = 12;
        cell.add_compartment(&compartment);
        cell.add_compartment(&compartment2);
        Cell cell2(2,2);
        cell2.add_compartment(&compartment);
        cell2.add_compartment(&compartment2);
        cell2.add_compartment(&compartment2);
        Cell cell42(42, 42);
        std::vector<double> compartments {34.1, 55.21, 3.141592, 44, 2124, 42.42};
        for(double& elem: compartments) {
            cell42.add_compartment(&elem);
        }
        WHEN("We record some data and prepare the dataset for a big enough max buffer size") {
            std::shared_ptr<cells_t> cells = std::make_shared<cells_t>(
                    std::initializer_list<cells_t::value_type>{{1, cell}, {2, cell2}, {42, cell42}});

            int num_steps = 3;
            size_t max_buffer_size = 1024;
            std::shared_ptr<ReportFormat> format = ReportFormat::create_ReportFormat(
                    "test_reportformat", max_buffer_size, num_steps, cells, SONATA);
            std::array<int, 3> cellids = {1, 42};

            format->prepare_dataset();
            for (int i = 0; i < num_steps; i++) {
                format->record_data(0, cellids.size(), &cellids[0]);
                format->update_timestep(0.0);
            }
            format->write_data();
            format->close();

            THEN("The buffer size is the total number of steps times the total number of compartments") {
                // 1024 / sizeof(float) / 11 = 23 > 3 (total number of steps)
                // buffer_size = 11 * 3
                REQUIRE(format->get_buffer_size() == 33);
            }

            THEN("We check the node ids of the sonata report") {
                SonataFormat* sonata = dynamic_cast<SonataFormat*>(format.get());
                const std::vector<uint64_t> node_ids = sonata->get_node_ids();
                std::vector<uint64_t> compare = { 1, 2, 42 };
                REQUIRE(node_ids == compare);
            }

            THEN("We check the compartment ids of the sonata report") {
                SonataFormat* sonata = dynamic_cast<SonataFormat*>(format.get());
                const std::vector<uint32_t> element_ids = sonata->get_element_ids();
                std::vector<uint32_t> compare = { 1000, 1001, 2000, 2001, 2002, 42000, 42001,
                                                  42002, 42003, 42004, 42005 };
                REQUIRE(element_ids == compare);
            }

            THEN("We check the index pointers of the sonata report") {
                SonataFormat* sonata = dynamic_cast<SonataFormat*>(format.get());
                const std::vector<uint64_t> index_pointers = sonata->get_index_pointers();
                std::vector<uint64_t> compare = { 0, 2, 5 };
                REQUIRE(index_pointers == compare);
            }
        }
        WHEN("We record some other data and prepare the dataset for a small max buffer size") {
            std::shared_ptr<cells_t> cells = std::make_shared<cells_t>(
                    std::initializer_list<cells_t::value_type>{{1, cell}, {2, cell2}, {42, cell42}});

            int num_steps = 3;
            size_t max_buffer_size = 128;
            std::shared_ptr<ReportFormat> format2 = ReportFormat::create_ReportFormat(
                    "test_reportformat2", max_buffer_size, num_steps, cells, SONATA);
            std::array<int, 3> cellids = {1, 42};

            format2->prepare_dataset();
            for (int i = 0; i < num_steps; i++) {
                format2->record_data(i, cellids.size(), &cellids[0]);
                format2->update_timestep(i);
            }
            format2->write_data();
            format2->close();

            THEN("The buffer size is the number of steps to write that fit on the buffer times the total compartments") {
                // 128 / sizeof(float) / 11 = 2
                // buffer_size = 11 * 2
                REQUIRE(format2->get_buffer_size() == 22);
            }
        }
    }

}
