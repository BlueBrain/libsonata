#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <bbp/sonata/reports.h>
#include <reports/utils/logger.h>

struct Neuron {
    uint64_t node_id;
    std::string kind;  // soma / element
    std::vector<double> voltages;
};

void generate_spikes(const std::vector<uint64_t>& nodeids,
                     std::vector<double>& spike_timestamps,
                     std::vector<int>& spike_node_ids,
                     double tstart,
                     double tstop,
                     int seed,
                     int max_size) {
    // Generate 10,30,50,70,90 spikes
    size_t num_spikes = (10 + (20 * seed)) % 100;
    spike_timestamps.reserve(num_spikes);
    spike_node_ids.reserve(num_spikes);
    for (size_t i = 0; i < num_spikes; i++) {
        // timestamp between tstart and tstop
        double timestamp = tstart + (0.5 + seed) / (max_size / (tstop - tstart));
        // get an index to the nodeids
        size_t index = seed % nodeids.size();
        uint32_t node_id = nodeids[index];
        spike_timestamps.push_back(timestamp);
        spike_node_ids.push_back(node_id);
    }
}

void generate_elements(Neuron& neuron, int seed) {
    // 50+-5 elements
    size_t num_elements = 50 + ((seed % 10) - 5);
    if (neuron.kind == "soma") {
        num_elements = 1;
    }
    neuron.voltages.reserve(num_elements);
    for (size_t j = 0; j < num_elements; j++) {
        neuron.voltages.push_back(seed % 10);
    }
}

std::vector<uint64_t> generate_data(std::vector<Neuron>& neurons,
                                    const std::string& kind,
                                    int seed) {
    std::vector<uint64_t> nodeids;
    // Each nodeid starts with the rank*10 (i.e. rank 5 will have nodeids: 51, 52, 53...)
    uint64_t next_nodeid = 1 + seed * 10;

    // 5+-5 neurons
    uint32_t num_neurons = 5 + ((2 + (seed % 10)) - 5);
    nodeids.reserve(num_neurons);
    for (uint32_t i = 0; i < num_neurons; i++) {
        Neuron tmp_neuron;
        tmp_neuron.kind = kind;

        nodeids.push_back(next_nodeid);
        tmp_neuron.node_id = next_nodeid++;

        // element or soma
        generate_elements(tmp_neuron, seed);
        neurons.push_back(tmp_neuron);
    }
    return nodeids;
}

void init(const char* report_name,
          const char* population_name,
          double tstart,
          double tstop,
          double dt,
          std::vector<Neuron>& neurons,
          const std::string& kind) {
    // logic for registering soma and element reports with reportinglib
    sonata_create_report(report_name, tstart, tstop, dt, kind.c_str());
    for (auto& neuron : neurons) {
        sonata_add_node(report_name, population_name, neuron.node_id);
        int element_id = neuron.node_id * 1000;

        for (auto& element : neuron.voltages) {
            sonata_add_element(report_name, population_name, neuron.node_id, element_id, &element);
            ++element_id;
        }
    }
}

void change_data(std::vector<Neuron>& neurons) {
    // Increment in 1 per timestep every voltage
    for (auto& neuron : neurons) {
        for (auto& element : neuron.voltages) {
            element++;
        }
    }
}

void print_data(std::vector<Neuron>& neurons) {
    for (auto& neuron : neurons) {
        std::cout << "++NEURON node_id: " << neuron.node_id << "\nelements:\n";
        std::copy(neuron.voltages.begin(),
                  neuron.voltages.end(),
                  std::ostream_iterator<double>(std::cout, ", "));
        std::cout << "\n\n";
    }
}

int main() {
    logger->set_level(spdlog::level::trace);
    int global_rank = 0;
    int global_size = 1;
#ifdef HAVE_MPI
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &global_size);
#endif
    if (global_rank == 0) {
        logger->info("Starting...");
    }

    const double dt = 0.1;
    const double tstart = 0.0;
    const double tstop = 0.3;

    std::vector<Neuron> element_neurons;
    std::vector<Neuron> soma_neurons;
    std::vector<uint64_t> element_nodeids;
    std::vector<uint64_t> soma_nodeids;
    std::vector<double> spike_timestamps;
    std::vector<int> spike_node_ids;

    // Each rank will get different number of nodes (some even 0, so will be idle ranks)
    element_nodeids = generate_data(element_neurons, "compartment", global_rank);
    soma_nodeids = generate_data(soma_neurons, "soma", global_rank);
    generate_spikes(
        soma_nodeids, spike_timestamps, spike_node_ids, tstart, tstop, global_rank, global_size);

    std::vector<int> int_element_nodeids(begin(element_nodeids), end(element_nodeids));
    std::vector<int> int_soma_nodeids(begin(soma_nodeids), end(soma_nodeids));

    if (global_rank == 0) {
        logger->info("Initializing data structures (reports, nodes, elements)");
    }
    const char* element_report = "compartment_report";
    const char* soma_report = "soma_report";
    const char* population_name = "All";

    init(element_report, population_name, tstart, tstop, dt, element_neurons, "compartment");
    init(soma_report, population_name, tstart, tstop, dt, soma_neurons, "soma");
    sonata_set_max_buffer_size_hint(20);
    sonata_set_atomic_step(dt);

    sonata_setup_communicators();
    sonata_prepare_datasets();
    sonata_time_data();

    if (global_rank == 0) {
        logger->info("Starting the simulation!");
    }
    // Calculate number of steps of the simulation
    double sim_steps = (tstop - tstart) / dt;
    int num_steps = static_cast<int>(std::ceil(sim_steps));
    double t = 0.0;
    for (int i = 0; i < num_steps; i++) {
        if (global_rank == 0) {
            logger->info("Recording data for step = {}", i);
        }
        sonata_record_node_data(i, element_nodeids.size(), &int_element_nodeids[0], element_report);
        sonata_record_node_data(i, soma_nodeids.size(), &int_soma_nodeids[0], soma_report);
        // Also works
        // sonata_rec(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Update timestep on reportinglib
        sonata_check_and_flush(t);
        t += dt;
        // Change data every timestep
        change_data(element_neurons);
        change_data(soma_neurons);
    }
    sonata_flush(t);
    const std::string output_dir = ".";
    // Write the spikes
    sonata_write_spikes(population_name,
                        spike_timestamps.data(),
                        spike_timestamps.size(),
                        spike_node_ids.data(),
                        spike_node_ids.size(),
                        output_dir.data());

    if (global_rank == 0) {
        logger->info("Finalizing...");
    }


#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 0;
}
