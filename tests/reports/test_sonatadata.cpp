#include <memory>
#include <iostream>
#include <catch2/catch.hpp>
#include <reports/data/sonata_data.hpp>

SCENARIO( "Test SonataData class", "[SonataData][IOWriter]" ) {
    GIVEN( "A node map structure" ) {
        using nodes_t = std::map<uint64_t, Node>;
        Node node(1,1);
        double element = 10;
        double element2 = 12;
        node.add_element(&element, 0);
        node.add_element(&element2, 1);
        Node node2(2,2);
        node2.add_element(&element, 10);
        node2.add_element(&element2, 11);
        node2.add_element(&element2, 12);
        Node node42(42, 42);
        std::vector<double> elements {34.1, 55.21, 3.141592, 44, 2124, 42.42};
        int i = 20;
        for(double& elem: elements) {
            node42.add_element(&elem, i);
            ++i;
        }
        WHEN("We record some data and prepare the dataset for a big enough max buffer size") {
            std::shared_ptr<nodes_t> nodes = std::make_shared<nodes_t>(
                    std::initializer_list<nodes_t::value_type>{{1, node}, {2, node2}, {42, node42}});

            int num_steps = 3;
            size_t max_buffer_size = 1024;
            std::shared_ptr<SonataData> sonata = std::make_shared<SonataData>("test_sonatadata", max_buffer_size, num_steps, nodes);
            std::vector<uint64_t> nodeids = {1, 42};

            sonata->prepare_dataset(false);
            for (int i = 0; i < num_steps; i++) {
                sonata->record_data(0, nodeids, 1);
                sonata->update_timestep(0.0);
            }
            sonata->write_data();
            sonata->close();

            THEN("The buffer size is the total number of steps times the total number of elements") {
                // 1024 / sizeof(float) / 11 = 23 > 3 (total number of steps)
                // buffer_size = 11 * 3
                REQUIRE(sonata->get_buffer_size() == 33);
            }

            THEN("We check the node ids of the sonata report") {
                const std::vector<uint64_t> node_ids = sonata->get_node_ids();
                std::vector<uint64_t> compare = { 1, 2, 42 };
                REQUIRE(node_ids == compare);
            }

            THEN("We check the element ids of the sonata report") {
                const std::vector<uint32_t> element_ids = sonata->get_element_ids();
                /*std::vector<uint32_t> compare = { 1000, 1001, 2000, 2001, 2002, 42000, 42001,
                                                  42002, 42003, 42004, 42005 };*/
                std::vector<uint32_t> compare = { 0, 1, 10, 11, 12, 20, 21,
                                                  22, 23, 24, 25 };
                REQUIRE(element_ids == compare);
            }

            THEN("We check the index pointers of the sonata report") {
                const std::vector<uint64_t> index_pointers = sonata->get_index_pointers();
                std::vector<uint64_t> compare = { 0, 2, 5 };
                REQUIRE(index_pointers == compare);
            }
        }
        WHEN("We record some other data and prepare the dataset for a small max buffer size") {
            std::shared_ptr<nodes_t> nodes = std::make_shared<nodes_t>(
                    std::initializer_list<nodes_t::value_type>{{1, node}, {2, node2}, {42, node42}});

            int num_steps = 3;
            size_t max_buffer_size = 128;
            std::shared_ptr<SonataData> sonata2 = std::make_shared<SonataData>("test_sonatadata2", max_buffer_size, num_steps, nodes);
            std::vector<uint64_t> nodeids = {1, 42};

            sonata2->prepare_dataset(false);
            for (int i = 0; i < num_steps; i++) {
                sonata2->record_data(i, nodeids, 1);
                sonata2->update_timestep(i);
            }
            sonata2->write_data();
            sonata2->close();

            THEN("The buffer size is the number of steps to write that fit on the buffer times the total elements") {
                // 128 / sizeof(float) / 11 = 2
                // buffer_size = 11 * 2
                REQUIRE(sonata2->get_buffer_size() == 22);
            }
        }
    }

}
