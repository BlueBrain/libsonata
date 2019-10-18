#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <reports/utils/logger.hpp>
#include <bbp/reports/records.h>

struct Neuron {
    int node_id;
    std::string kind; // soma / element
    std::vector<double> voltages;
    std::vector<double> spike_timestamps;
};

const double dt = 0.1;
const double tstart = 0.0;
const double tstop = 0.3;

void generate_spikes(Neuron& neuron) {

    int num_spikes = 0;
    int rand = std::rand()%60;
    if (rand > 45) {
        num_spikes = 2;
    } else if(rand > 20) {
        num_spikes = 1;
    } // Otherwise 0

    neuron.spike_timestamps.reserve(num_spikes);
    for(int i=0; i<num_spikes; i++) {
        // timestamp between tstart and tstop
        double timestamp = tstart + static_cast<double> (std::rand()) / (static_cast<double> (RAND_MAX/(tstop-tstart)));
        neuron.spike_timestamps.push_back(timestamp);
    }
}

void generate_elements(Neuron& neuron) {

    // 50+-5 elements
    int num_elements = 50 + ((std::rand() % 10) - 5);

    if(neuron.kind == "soma"){
        num_elements = 1;
    }

    neuron.voltages.reserve(num_elements);
    for (int j = 0; j < num_elements; j++) {
        neuron.voltages.push_back(std::rand() % 10);
    }
}

std::vector<uint64_t> generate_data(std::vector<Neuron>& neurons, const std::string& kind, int seed) {

    // Set random seed for reproducibility
    std::srand(static_cast<unsigned int>(23487 * (seed + 1)));

    std::vector<uint64_t> nodeids;
    // Each gid starts with the rank*10 (i.e. rank 5 will have gids: 51, 52, 53...)
    uint64_t nextgid = 1 + seed*10;
    uint32_t c_id = 0;

    // 5+-5 neurons
    int num_neurons = 5 + ((std::rand() % 10) - 5);
    nodeids.reserve(num_neurons);
    for (int i = 0; i < num_neurons; i++) {
        Neuron tmp_neuron;
        tmp_neuron.kind = kind;

        nodeids.push_back(nextgid);
        tmp_neuron.node_id = nextgid++;

        if (tmp_neuron.kind == "spike") {
            generate_spikes(tmp_neuron);
        } else {
            // element or soma
            generate_elements(tmp_neuron);
        }

        neurons.push_back(tmp_neuron);
    }

    return nodeids;
}

void init(const char* report_name, std::vector<Neuron>& neurons) {

    // logic for registering soma and element reports with reportinglib
    for (auto& neuron : neurons) {
        records_add_report(report_name, neuron.node_id, neuron.node_id, neuron.node_id, tstart, tstop, dt,
                0, neuron.kind.c_str(), 0, nullptr);

        const int mapping_size = 1;
        int element_id = neuron.node_id*1000;
        int mapping[mapping_size] = {element_id};

        for (auto& element: neuron.voltages) {
            records_add_var_with_mapping(report_name, neuron.node_id, &element, mapping_size, mapping);
            mapping[0] = ++element_id;
        }

        for (auto& spike: neuron.spike_timestamps) {
            records_add_var_with_mapping(report_name, neuron.node_id, &spike, mapping_size, mapping);
        }
    }
}

void change_data(std::vector<Neuron>& neurons) {

    // Increment in 1 per timestep every voltage
    for (auto& neuron : neurons) {
        for (auto& element: neuron.voltages) {
            element++;
        }
    }
}

void print_data(std::vector<Neuron>& neurons) {

    for (auto& neuron : neurons) {
        std::cout << "++NEURON node_id: " << neuron.node_id << std::endl;
        std::cout << "elements:" << std::endl;
        for (auto& element: neuron.voltages) {
            std::cout << element << ", ";
        }
        std::cout << std::endl << std::endl;
        std::cout << "Spikes:" << std::endl;
        for (auto& spike: neuron.spike_timestamps) {
            std::cout << spike << ", ";
        }
        std::cout << std::endl;
    }
}

int main() {
    logger->set_level(spdlog::level::trace);
    int global_rank = 0;
#ifdef HAVE_MPI
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
#endif
    if(global_rank == 0) {
        logger->info("Starting...");
    }
    std::vector<Neuron> element_neurons;
    std::vector<Neuron> soma_neurons;
    std::vector<Neuron> spike_neurons;
    std::vector<uint64_t> element_nodeids;
    std::vector<uint64_t> soma_nodeids;
    std::vector<uint64_t> spike_nodeids;

    // Each rank will get different number of nodes (some even 0, so will be idle ranks)
    element_nodeids = generate_data(element_neurons, "compartment", global_rank);
    soma_nodeids = generate_data(soma_neurons, "soma", global_rank);
    spike_nodeids = generate_data(spike_neurons, "spike", global_rank);

    std::vector<int> int_element_nodeids(begin(element_nodeids), end(element_nodeids));
    std::vector<int> int_soma_nodeids(begin(soma_nodeids), end(soma_nodeids));


    if(global_rank == 0) {
        logger->info("Initializing data structures (reports, nodes, elements)");
    }
    const char* element_report = "compartment_report";
    const char* soma_report = "soma_report";
    const char* spike_report = "spike_report";

    init(element_report, element_neurons);
    init(soma_report, soma_neurons);
    init(spike_report, spike_neurons);
    records_set_max_buffer_size_hint(20);
    records_set_atomic_step(dt);

    records_setup_communicator();
    records_finish_and_share();

    records_time_data();

    if(global_rank == 0) {
        logger->info("Starting the simulation!");
    }
    // Calculate number of steps of the simulation
    int num_steps = static_cast<int>((tstop - tstart) / dt);
    if (std::fabs(num_steps * dt + tstart - tstop) > std::numeric_limits<float>::epsilon()) {
        num_steps++;
    }
    double t = 0.0;
    for(int i=0; i<num_steps; i++) {
        if(global_rank == 0) {
            logger->info("Recording data for step = {}", i);
        }
        records_nrec(i, element_nodeids.size(), &int_element_nodeids[0], element_report);
        records_nrec(i, soma_nodeids.size(), &int_soma_nodeids[0], soma_report);
        // Also works
        // records_rec(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Update timestep on reportinglib
        records_end_iteration(t);
        t += dt;
        // Change data every timestep
        change_data(element_neurons);
        change_data(soma_neurons);
    }
    records_flush(t);

    if(global_rank == 0) {
        logger->info("Finalizing...");
    }
#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 0;
}
