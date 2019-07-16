#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <mpi.h>

#include <bbp/reports/records.hpp>

struct Neuron {
    // some properties
    int gid;
    std::string kind; // soma / compartment
    std::vector<double> compartments;
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

void generate_compartments(Neuron& neuron) {

    // 15+-5 compartments
    int num_compartments = 50 + ((std::rand() % 10) - 5);

    if(neuron.kind == "soma"){
        num_compartments = 1;
    }

    neuron.compartments.reserve(num_compartments);
    for (int j = 0; j < num_compartments; j++) {
        neuron.compartments.push_back(std::rand() % 10);
    }
}

std::vector<int> generate_data(std::vector<Neuron>& neurons, const std::string& kind, int seed) {

    // Set random seed for reproducibility
    std::srand(static_cast<unsigned int>(23487 * (seed + 1)));

    std::vector<int> cellids;
    // Each gid starts with the rank*10 (i.e. rank 5 will have gids: 51, 52, 53...)
    uint64_t nextgid = 1 + seed*10;
    uint32_t c_id = 0;

    // 5+-5 neurons
    int num_neurons = 5 + ((std::rand() % 10) - 5);
    cellids.reserve(num_neurons);
    for (int i = 0; i < num_neurons; i++) {
        Neuron tmp_neuron;
        tmp_neuron.kind = kind;

        cellids.push_back(nextgid);
        tmp_neuron.gid = nextgid++;

        if (tmp_neuron.kind == "spike") {
            generate_spikes(tmp_neuron);
        } else {
            // compartment or soma
            generate_compartments(tmp_neuron);
        }

        neurons.push_back(tmp_neuron);
    }

    return cellids;
}

void init(const char* report_name, std::vector<Neuron>& neurons) {

    // logic for registering soma and compartment reports with reportinglib
    for (auto& neuron : neurons) {
        records_add_report(report_name, neuron.gid, neuron.gid, neuron.gid, tstart, tstop, dt, neuron.kind.c_str());

        for (auto& compartment: neuron.compartments) {
            records_add_var_with_mapping(report_name, neuron.gid, &compartment);
        }

        for (auto& spike: neuron.spike_timestamps) {
            records_add_var_with_mapping(report_name, neuron.gid, &spike);
        }
    }
}

void change_data(std::vector<Neuron>& neurons) {

    // Increment in 1 per timestep every voltage
    for (auto& neuron : neurons) {
        for (auto& compartment: neuron.compartments) {
            compartment++;
        }
    }
}

void print_data(std::vector<Neuron>& neurons) {

    for (auto& neuron : neurons) {
        std::cout << "++NEURON GID: " << neuron.gid << std::endl;
        std::cout << "Compartments:" << std::endl;
        for (auto& compartment: neuron.compartments) {
            std::cout << compartment << ", ";
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
    std::cout << "+++++++MPI_INIT" << std::endl;
    MPI_Init(nullptr, nullptr);
    int global_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);

    std::vector<Neuron> compartment_neurons;
    std::vector<Neuron> soma_neurons;
    std::vector<Neuron> spike_neurons;
    std::vector<int> compartment_cellids;
    std::vector<int> soma_cellids;
    std::vector<int> spike_cellids;

    std::cout << "+++++++Generating Data" << std::endl;
    // Each rank will get different number of cells (some even 0, so will be idle ranks)
    compartment_cellids = generate_data(compartment_neurons, "compartment", global_rank);
    soma_cellids = generate_data(soma_neurons, "soma", global_rank);
    spike_cellids = generate_data(spike_neurons, "spike", global_rank);

    // std::cout << "+++++++Printing Data" << std::endl;
    // print_data(spike_neurons);

    std::cout << "++++++++Initializing report/cell/compartment structure" << std::endl;
    const char* compartment_report = "compartment_report";
    const char* soma_report = "soma_report";
    const char* spike_report = "spike_report";

    init(compartment_report, compartment_neurons);
    init(soma_report, soma_neurons);
    init(spike_report, spike_neurons);
    records_set_max_buffer_size_hint(20);

    std::cout << "++++++++++Prepare datasets" << std::endl;
    records_setup_communicator();
    records_finish_and_share();

    std::cout << "++++++++++Starting simulation!" << std::endl;
    // Calculate number of steps of the simulation
    int num_steps = (int)std::floor((tstop - tstart) / dt);
    if (std::fabs(num_steps * dt + tstart - tstop) > 1e-9) {
        num_steps++;
    }
    double t = 0.0;
    for(int i=0; i<num_steps; i++) {
        std::cout << "++++Recording data for t = " << t << std::endl;
        records_nrec(t, compartment_cellids.size(), &compartment_cellids[0], compartment_report);
        records_nrec(t, soma_cellids.size(), &soma_cellids[0], soma_report);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Update timestep on reportinglib
        records_end_iteration(t);
        t += dt;
        // Change data every timestep
        change_data(compartment_neurons);
        change_data(soma_neurons);
    }
    records_flush(t);

    MPI_Finalize();
    return 0;
}
