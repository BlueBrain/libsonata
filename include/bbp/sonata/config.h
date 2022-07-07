/*************************************************************************
 * Copyright (C) 2018-2021 Blue Brain Project
 *                         Jonas Karlsson <jonas.karlsson@epfl.ch>
 *                         Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include <memory>  // std::unique_ptr
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <bbp/sonata/edges.h>
#include <bbp/sonata/nodes.h>
#include <bbp/sonata/optional.hpp>

#include "common.h"
#include "optional.hpp"
#include "variant.hpp"


namespace bbp {
namespace sonata {

using variantValueType = nonstd::variant<bool, std::string, int, double>;

/**
 * Stores population-specific network information.
 */
struct SONATA_API PopulationProperties {
    /**
     * Population type
     */
    std::string type;

    /**
     * Path to the template HOC files defining the E-Mode
     */
    std::string biophysicalNeuronModelsDir;

    /**
     * Path to the directory containing the morphologies
     */
    std::string morphologiesDir;

    /**
     * Dictionary for alternate directory paths.
     */
    std::unordered_map<std::string, std::string> alternateMorphologyFormats;
};

/**
 *  Read access to a SONATA circuit config file.
 */
class SONATA_API CircuitConfig
{
  public:
    /**
     * Parses a SONATA JSON config file.
     *
     * \throws SonataError on:
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    CircuitConfig(const std::string& contents, const std::string& basePath);

    /**
     * Loads a SONATA JSON config file from disk and returns a CircuitConfig object which
     * parses it.
     *
     * \throws SonataError on:
     *          - Non accesible file (does not exists / does not have read access)
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     *          - Missing entries which become mandatory when another entry is present
     *          - Multiple populations with the same name in different edge/node networks
     */
    static CircuitConfig fromFile(const std::string& path);

    /**
     * Returns the path to the node sets file.
     */
    const std::string& getNodeSetsPath() const;

    /**
     *  Returns a set with all available population names across all the node networks.
     */
    std::set<std::string> listNodePopulations() const;

    /**
     * Creates and returns a NodePopulation object, initialized from the given population,
     * and the node network it belongs to.
     *
     * \throws SonataError if the given population does not exist in any node network.
     */
    NodePopulation getNodePopulation(const std::string& name) const;

    /**
     * Returns a set with all available population names across all the edge networks.
     */
    std::set<std::string> listEdgePopulations() const;

    /**
     * Creates and returns an EdgePopulation object, initialized from the given population,
     * and the edge network it belongs to.
     *
     * \throws SonataError if the given population does not exist in any edge network.
     */
    EdgePopulation getEdgePopulation(const std::string& name) const;

    /**
     * Return a structure containing node population specific properties, falling
     * back to network properties if there are no population-specific ones.
     *
     * \throws SonataError if the given population name does not correspond to any existing
     *         node population.
     */
    PopulationProperties getNodePopulationProperties(const std::string& name) const;

    /**
     * Return a structure containing edge population specific properties, falling
     * back to network properties if there are no population-specific ones.
     *
     * \throws SonataError if the given population name does not correspond to any existing
     *         edge population.
     */
    PopulationProperties getEdgePopulationProperties(const std::string& name) const;

    /**
     * Returns the configuration file JSON whose variables have been expanded by the
     * manifest entries.
     */
    const std::string& getExpandedJSON() const;

  private:
    struct Components {
        std::string morphologiesDir;
        std::unordered_map<std::string, std::string> alternateMorphologiesDir;
        std::string biophysicalNeuronModelsDir;
    };

    struct SubnetworkFiles {
        std::string elements;
        std::string types;
        std::set<std::string> populations;
    };

    class Parser;
    friend class Parser;
    class PopulationResolver;
    friend class PopulationResolver;

    // Circuit config json string whose paths and variables have been expanded to include
    // manifest variables
    std::string _expandedJSON;

    // Path to the nodesets file
    std::string _nodeSetsFile;

    // Default components value for all networks
    Components _components;

    // Nodes network paths
    std::vector<SubnetworkFiles> _networkNodes;
    // Node populations that override default components variables
    std::unordered_map<std::string, PopulationProperties> _nodePopulationProperties;

    // Edges network paths
    std::vector<SubnetworkFiles> _networkEdges;
    // Edge populations that override default components variables
    std::unordered_map<std::string, PopulationProperties> _edgePopulationProperties;
};

/**
 *  Read access to a SONATA simulation config file.
 */
class SONATA_API SimulationConfig
{
  public:
    /**
     * Parameters defining global simulation settings for spike reports
     */
    struct Run {
        enum class SpikeLocation { invalid = -1, soma, AIS };
        enum class IntegrationMethod { invalid = -1, euler, nicholson, nicholson_ion };

        /// Biological simulation end time in milliseconds
        double tstop{};
        /// Integration step duration in milliseconds
        double dt{};
        /// Random seed
        int randomSeed{};
        /// The spike detection threshold. Default is -30mV
        int spikeThreshold;
        /// The spike detection location. Can be either ‘soma’ or 'AIS'. Default is 'soma'
        SpikeLocation spikeLocation;
        /// Selects the NEURON/CoreNEURON integration method. This parameter sets the NEURON
        /// global variable h.secondorder. Default 0 ('euler')
        IntegrationMethod integrationMethod;
        /// Run without Stimulus or Reports for given duration prior to t=0
        /// using a timestep computed as dt=0.1*forward_skip. Default = None
        nonstd::optional<int> forwardSkip{nonstd::nullopt};
    };
    /**
     * Parameters to override simulator output for spike reports
     */
    struct Output {
        enum class SpikesSortOrder { invalid = -1, none, by_id, by_time };

        /// Spike report file output directory. Default is "output"
        std::string outputDir;
        /// Filename where console output is written. Default is STDOUT.
        std::string logFile;
        /// Spike report file name. Default is "out.h5"
        std::string spikesFile;
        /// The sorting order of the spike report. Default is "by_time"
        SpikesSortOrder sortOrder;
    };
    /**
     * Parameters defining global experimental conditions.
     */
    struct Conditions {
        /// Temperature of experiment. Default is 34.0
        double celsius;
        /// Initial membrane voltage in mV. Default is -80
        double vInit;
        /// Synapse at start of simulation are in depleted state. Default is false
        bool synapsesInitDepleted;
        /// Extracellular calcium concentration, being applied to the synapse uHill parameter in
        /// order to scale the U parameter of synapses. Default is None.
        nonstd::optional<double> extracellularCalcium{nonstd::nullopt};
        /// Limit spontaneous release to single vesicle when true. Default is false
        bool minisSingleVesicle;
        /// Enable legacy behavior to randomize the GABA_A rise time in the helper functions.
        /// Default is false
        bool randomizeGabaRiseTime;
        /// Properties to assign values to variables in synapse MOD files.
        /// The format is a dictionary with keys being the SUFFIX names and values being
        /// dictionaries of variables' names and values.
        std::unordered_map<std::string, std::unordered_map<std::string, variantValueType>>
            mechanisms;
    };
    /**
     * List of report parameters collected during the simulation
     */
    struct Report {
        enum class Sections { invalid = -1, soma, axon, dend, apic, all };
        enum class Type { invalid = -1, compartment, summation, synapse };
        enum class Scaling { invalid = -1, none, area };
        enum class Compartments { invalid = -1, center, all };

        /// Node sets on which to report
        std::string cells;
        /// Sections on which to report. Default value: "soma"
        Sections sections;
        /// Report type.
        Type type;
        /// For summation type, specify the handling of density values.
        /// Default value: "area"
        Scaling scaling;
        /// For compartment type, select compartments to report.
        /// Default value: "center"(for sections: soma), "all"(for other sections)
        Compartments compartments;
        /// The simulation variable to access
        std::string variableName;
        /// Descriptive text of the unit recorded. Not validated for correctness
        std::string unit;
        /// Interval between reporting steps in milliseconds
        double dt{};
        /// Time to step reporting in milliseconds
        double startTime{};
        /// Time to stop reporting in milliseconds
        double endTime{};
        /// Report filename. Default is "<report name>_SONATA.h5"
        std::string fileName;
        /// Allows for supressing a report so that is not created. Default is true
        bool enabled = true;
    };

    using ReportMap = std::unordered_map<std::string, Report>;

    struct InputBase {
        enum class Module {
            invalid = -1,
            linear,
            relative_linear,
            pulse,
            subthreshold,
            hyperpolarizing,
            synapse_replay,
            seclamp,
            noise,
            shot_noise,
            relative_shot_noise,
            absolute_shot_noise,
            ornstein_uhlenbeck,
            relative_ornstein_uhlenbeck
        };

        enum class InputType {
            invalid = -1,
            spikes,
            extracellular_stimulation,
            current_clamp,
            voltage_clamp,
            conductance
        };

        /// Type of stimulus
        Module module;
        /// Type of input
        InputType inputType;
        /// Time when input is activated (ms)
        double delay{};
        /// Time duration for how long input is activated (ms)
        double duration{};
        /// Node set which is affected by input
        std::string nodeSet;
    };

    struct InputLinear: public InputBase {
        /// The amount of current initially injected (nA)
        double ampStart{};
        /// The final current when a stimulus concludes (nA)
        double ampEnd{};
    };

    struct InputRelativeLinear: public InputBase {
        /// The percentage of a cell's threshold current to inject
        double percentStart{};
        /// The percentage of a cell's threshold current to inject at the end
        double percentEnd{};
    };

    struct InputPulse: public InputBase {
        /// The amount of current initially injected (nA)
        double ampStart{};
        /// The final current when a stimulus concludes (nA)
        double ampEnd{};
        /// The length of time each pulse lasts (ms)
        double width{};
        /// The frequency of pulse trains (Hz)
        double frequency{};
    };

    struct InputSubthreshold: public InputBase {
        /// A percentage adjusted from 100 of a cell's threshold current
        double percentLess{};
    };

    struct InputHyperpolarizing: public InputBase {};

    struct InputSynapseReplay: public InputBase {
        /// The location of the file with the spike info for injection
        std::string spikeFile;
        /// The node set to replay spikes from
        std::string source;
    };

    struct InputSeclamp: public InputBase {
        /// The membrane voltage the targeted cells should be held at (mV)
        double voltage{};
    };

    struct InputNoise: public InputBase {
        /// The mean value of current to inject (nA), default = None
        nonstd::optional<double> mean{nonstd::nullopt};
        /// The mean value of current to inject as a percentage of threshold current, default = None
        nonstd::optional<double> meanPercent{nonstd::nullopt};
        /// State var to track whether the value of injected noise current is mean or
        /// mean_percent
        double variance{};
    };

    struct InputShotNoise: public InputBase {
        /// The rise time of the bi-exponential shots (ms)
        double riseTime{};
        /// The decay time of the bi-exponential shots (ms)
        double decayTime{};
        /// Override the random seed to introduce correlations between cells
        int randomSeed{};
        /// Timestep of generated signal in ms. Default is 0.25 ms
        double dt{};
        /// Rate of Poisson events (Hz)
        int rate{};
        /// The mean of gamma-distributed amplitudes in nA (current_clamp) or uS (conductance)
        double ampMean{};
        /// The variance of gamma-distributed amplitudes in nA^2 (current_clamp) or uS^2
        /// (conductance)
        double ampVar{};
    };

    struct InputRelativeShotNoise: public InputBase {
        /// The rise time of the bi-exponential shots (ms)
        double riseTime{};
        /// The decay time of the bi-exponential shots (ms)
        double decayTime{};
        /// Override the random seed to introduce correlations between cells
        int randomSeed{};
        /// Timestep of generated signal in ms. Default is 0.25 ms
        double dt{};
        /// The coefficient of variation (sd/mean) of gamma-distributed amplitudes
        double ampCv{};
        /// Signal mean as percentage of a cell’s threshold current (current_clamp) or inverse input
        /// resistance (conductance)
        double meanPercent{};
        /// signal std dev as percentage of a cell’s threshold current (current_clamp) or inverse
        /// input resistance (conductance).
        double sdPercent{};
    };

    struct InputAbsoluteShotNoise: public InputBase {
        /// The rise time of the bi-exponential shots (ms)
        double riseTime{};
        /// The decay time of the bi-exponential shots (ms)
        double decayTime{};
        /// Override the random seed to introduce correlations between cells
        int randomSeed{};
        /// Timestep of generated signal in ms. Default is 0.25 ms
        double dt{};
        /// The coefficient of variation (sd/mean) of gamma-distributed amplitudes
        double ampCv{};
        /// Signal mean in nA (current_clamp) or uS (conductance).
        double mean{};
        /// signal std dev in nA (current_clamp) or uS (conductance).
        double sigma{};
    };

    struct InputOrnsteinUhlenbeck: public InputBase {
        /// Relaxation time constant in ms
        double tau{};
        /// Reversal potential for conductance injection in mV. Default is 0
        double reversal{};
        /// Timestep of generated signal in ms. Default is 0.25 ms
        double dt{};
        /// Override the random seed to introduce correlations between cells
        int randomSeed{};
        /// Signal mean in nA (current_clamp) or uS (conductance)
        double mean{};
        /// Signal std dev in nA (current_clamp) or uS (conductance)
        double sigma{};
    };

    struct InputRelativeOrnsteinUhlenbeck: public InputBase {
        /// Relaxation time constant in ms
        double tau{};
        /// Reversal potential for conductance injection in mV. Default is 0
        double reversal{};
        /// Timestep of generated signal in ms. Default is 0.25 ms
        double dt{};
        /// Override the random seed to introduce correlations between cells
        int randomSeed{};
        /// Signal mean as percentage of a cell’s threshold current (current_clamp) or inverse input
        /// resistance (conductance)
        double meanPercent{};
        /// Signal std dev as percentage of a cell’s threshold current (current_clamp) or inverse
        /// input resistance (conductance)
        double sdPercent{};
    };

    using Input = nonstd::variant<InputLinear,
                                  InputRelativeLinear,
                                  InputPulse,
                                  InputSubthreshold,
                                  InputHyperpolarizing,
                                  InputSynapseReplay,
                                  InputSeclamp,
                                  InputNoise,
                                  InputShotNoise,
                                  InputRelativeShotNoise,
                                  InputAbsoluteShotNoise,
                                  InputOrnsteinUhlenbeck,
                                  InputRelativeOrnsteinUhlenbeck>;

    using InputMap = std::unordered_map<std::string, Input>;

    /**
     * List of connection parameters to adjust the synaptic strength or other properties of edges
     *  between two sets of nodes
     */
    struct ConnectionOverride {
        /// node_set specifying presynaptic nodes
        std::string source;
        /// node_set specifying postsynaptic nodes
        std::string target;
        /// Scalar to adjust synaptic strength, default = 1.
        double weight{1.};
        /// Rate to spontaneously trigger the synapses in this connection_override, default = None
        nonstd::optional<double> spontMinis{nonstd::nullopt};
        /// Snippet of hoc code to be executed on the synapses in this connection_override, default
        /// = None
        nonstd::optional<std::string> synapseConfigure{nonstd::nullopt};
        /// Synapse helper files to instantiate the synapses in this connection_override, default =
        /// None
        nonstd::optional<std::string> modoverride{nonstd::nullopt};
        /// Value to override the synaptic delay time originally set in the edge file (ms),
        /// default = None.
        nonstd::optional<double> synapseDelayOverride{nonstd::nullopt};
        /// Adjustments from weight of this connection_override are applied after the specified
        /// delay has elapsed in ms, default = 0.
        double delay{0.};
    };
    using ConnectionMap = std::unordered_map<std::string, ConnectionOverride>;

    enum class SimulatorType { invalid = -1, NEURON, CORENEURON };

    /**
     * Parses a SONATA JSON simulation configuration file.
     *
     * \throws SonataError on:
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     */
    SimulationConfig(const std::string& content, const std::string& basePath);

    /**
     * Loads a SONATA JSON simulation config file from disk and returns a CircuitConfig object
     * which parses it.
     *
     * \throws SonataError on:
     *          - Non accesible file (does not exists / does not have read access)
     *          - Ill-formed JSON
     *          - Missing mandatory entries (in any depth)
     */
    static SimulationConfig fromFile(const std::string& path);

    /**
     * Returns the base path of the simulation config file
     */
    const std::string& getBasePath() const noexcept;

    /**
     * Returns the JSON content of the simulation config file
     */
    const std::string& getJSON() const noexcept;

    /**
     * Returns circuit config file associated with this simulation config
     */
    const std::string& getNetwork() const noexcept;

    /**
     * Returns the Run section of the simulation configuration.
     */
    const Run& getRun() const noexcept;

    /**
     * Returns the Output section of the simulation configuration.
     */
    const Output& getOutput() const noexcept;

    /**
     * Returns the Conditions section of the simulation configuration.
     */
    const Conditions& getConditions() const noexcept;

    /**
     * Returns the names of the reports
     */
    std::set<std::string> listReportNames() const;

    /**
     * Returns the given report parameters.
     *
     * \throws SonataError if the given report name does not correspond with any existing
     *         report.
     */
    const Report& getReport(const std::string& name) const;

    /**
     * Returns the names of the reports
     */
    std::set<std::string> listInputNames() const;

    /**
     * Returns the given input parameters.
     *
     * \throws SonataError if the given input name does not exist
     */
    const Input& getInput(const std::string& name) const;

    /**
     * Returns the names of the connection_overrides
     */
    std::set<std::string> listConnectionOverrideNames() const;

    /**
     * Returns the given connection parameters
     *
     * \throws SonataError if the given connection name does not correspond with any existing
     *         connection.
     */
    const ConnectionOverride& getConnectionOverride(const std::string& name) const;

    /**
     * Returns the name of simulator, default = NEURON
     * \throws SonataError if the given value is neither NEURON nor CORENEURON
     */
    const SimulationConfig::SimulatorType& getTargetSimulator() const;

    /**
     * Returns the path of node sets file overriding node_sets_file provided in _network,
     * default is empty in case of no setting in _network
     */
    const std::string& getNodeSetsFile() const noexcept;

    /**
     * Returns the name of node set to be instantiated for the simulation, default = None
     */
    const nonstd::optional<std::string>& getNodeSet() const noexcept;

    /**
     * Returns the configuration file JSON whose variables have been expanded by the
     * manifest entries.
     */
    const std::string& getExpandedJSON() const;

  private:
    // JSON string
    std::string _expandedJSON;
    // Base path of the simulation config file
    std::string _basePath;

    // Run section
    Run _run;
    // Output section
    Output _output;
    // List of reports
    ReportMap _reports;
    // Conditions section
    Conditions _conditions;
    // Path of circuit config file for the simulation
    std::string _network;
    // List of inputs
    InputMap _inputs;
    // List of connections
    ConnectionMap _connections;
    // Name of simulator
    SimulatorType _targetSimulator;
    // Path of node sets file
    std::string _nodeSetsFile;
    // Name of node set
    nonstd::optional<std::string> _nodeSet{nonstd::nullopt};

    class Parser;
    friend class Parser;
};

}  // namespace sonata
}  // namespace bbp
