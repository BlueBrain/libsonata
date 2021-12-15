/*
  This file contains docstrings for use in the Python bindings.
  Do not edit! They were automatically extracted by pybind11_mkdoc.
 */

#define __EXPAND(x)                                      x
#define __COUNT(_1, _2, _3, _4, _5, _6, _7, COUNT, ...)  COUNT
#define __VA_SIZE(...)                                   __EXPAND(__COUNT(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1))
#define __CAT1(a, b)                                     a ## b
#define __CAT2(a, b)                                     __CAT1(a, b)
#define __DOC1(n1)                                       __doc_##n1
#define __DOC2(n1, n2)                                   __doc_##n1##_##n2
#define __DOC3(n1, n2, n3)                               __doc_##n1##_##n2##_##n3
#define __DOC4(n1, n2, n3, n4)                           __doc_##n1##_##n2##_##n3##_##n4
#define __DOC5(n1, n2, n3, n4, n5)                       __doc_##n1##_##n2##_##n3##_##n4##_##n5
#define __DOC6(n1, n2, n3, n4, n5, n6)                   __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6
#define __DOC7(n1, n2, n3, n4, n5, n6, n7)               __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6##_##n7
#define DOC(...)                                         __EXPAND(__EXPAND(__CAT2(__DOC, __VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


static const char *__doc_bbp_sonata_CircuitConfig = R"doc(Read access to a SONATA circuit config file.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_CircuitConfig =
R"doc(Parses a SONATA JSON config file.

Throws:
    SonataError on: - Ill-formed JSON - Missing mandatory entries (in
    any depth) - Missing entries which become mandatory when another
    entry is present - Multiple populations with the same name in
    different edge/node networks)doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_alternateMorphologiesDir = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_biophysicalNeuronModelsDir = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_morphologiesDir = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Parser = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_PopulationResolver = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_SubnetworkFiles = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_SubnetworkFiles_elements = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_SubnetworkFiles_populations = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_SubnetworkFiles_types = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_components = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_edgePopulationProperties = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_expandedJSON = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_fromFile =
R"doc(Loads a SONATA JSON config file from disk and returns a CircuitConfig
object which parses it.

Throws:
    SonataError on: - Non accesible file (does not exists / does not
    have read access) - Ill-formed JSON - Missing mandatory entries
    (in any depth) - Missing entries which become mandatory when
    another entry is present - Multiple populations with the same name
    in different edge/node networks)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getEdgePopulation =
R"doc(Creates and returns an EdgePopulation object, initialized from the
given population, and the edge network it belongs to.

Throws:
    SonataError if the given population does not exist in any edge
    network.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getEdgePopulationProperties =
R"doc(Return a structure containing edge population specific properties,
falling back to network properties if there are no population-specific
ones.

Throws:
    SonataError if the given population name does not correspond to
    any existing edge population.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getExpandedJSON =
R"doc(Returns the configuration file JSON whose variables have been expanded
by the manifest entries.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getNodePopulation =
R"doc(Creates and returns a NodePopulation object, initialized from the
given population, and the node network it belongs to.

Throws:
    SonataError if the given population does not exist in any node
    network.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getNodePopulationProperties =
R"doc(Return a structure containing node population specific properties,
falling back to network properties if there are no population-specific
ones.

Throws:
    SonataError if the given population name does not correspond to
    any existing node population.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getNodeSetsPath = R"doc(Returns the path to the node sets file.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_listEdgePopulations =
R"doc(Returns a set with all available population names across all the edge
networks.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_listNodePopulations =
R"doc(Returns a set with all available population names across all the node
networks.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_networkEdges = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_networkNodes = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_nodePopulationProperties = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_nodeSetsFile = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_data = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_ids = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_times = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation_EdgePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation_afferentEdges = R"doc(Return inbound edges for given node IDs.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_connectingEdges = R"doc(Return edges connecting two given nodes.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_efferentEdges = R"doc(Return outbound edges for given node IDs.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_source = R"doc(Name of source population extracted from 'source_node_id' dataset)doc";

static const char *__doc_bbp_sonata_EdgePopulation_sourceNodeIDs = R"doc(Return source node IDs for a given edge selection)doc";

static const char *__doc_bbp_sonata_EdgePopulation_target = R"doc(Name of target population extracted from 'target_node_id' dataset)doc";

static const char *__doc_bbp_sonata_EdgePopulation_targetNodeIDs = R"doc(Return target node IDs for a given edge selection)doc";

static const char *__doc_bbp_sonata_EdgePopulation_writeIndices = R"doc(Write bidirectional node->edge indices to EdgePopulation HDF5.)doc";

static const char *__doc_bbp_sonata_NodePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_NodePopulation_NodePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_NodePopulation_matchAttributeValues =
R"doc(Return selection of where attribute values match value

As per node_set predicates, ``value`` must be one of type:

* number H5T_IEEE_*LE, H5T_STD_*LE

* string H5T_C_S1

* bool H5T_STD_I8LE

* null invalid

Throws:
    if the attribute dtype is not comparable

Note: This does not match dynamics_params datasets)doc";

static const char *__doc_bbp_sonata_NodePopulation_matchAttributeValues_2 = R"doc(Like matchAttributeValues, but for vectors of values to match)doc";

static const char *__doc_bbp_sonata_NodePopulation_regexMatch =
R"doc(For named attribute, return a selection where the passed regular
expression matches)doc";

static const char *__doc_bbp_sonata_NodeSets = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_NodeSets =
R"doc(Create nodeset from JSON

See also: https://github.com/AllenInstitute/sonata/blob/master/docs/SO
NATA_DEVELOPER_GUIDE.md#node-sets-file

Note: floating point values aren't supported for comparison

Parameter ``content``:
    is the JSON node_sets value

Throws:
    if content cannot be parsed)doc";

static const char *__doc_bbp_sonata_NodeSets_NodeSets_2 = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_NodeSets_3 = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_fromFile = R"doc(Open a SONATA `node sets` file from a path */)doc";

static const char *__doc_bbp_sonata_NodeSets_impl = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_materialize =
R"doc(Return a selection corresponding to the node_set name

Parameter ``name``:
    is the name of the node_set rule to be evaluated

Parameter ``population``:
    is the population overwhich the returned selection will be valid)doc";

static const char *__doc_bbp_sonata_NodeSets_names = R"doc(Names of the node sets available)doc";

static const char *__doc_bbp_sonata_NodeSets_operator_assign = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_toJSON = R"doc(Return string version of node sets)doc";

static const char *__doc_bbp_sonata_Population = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationProperties = R"doc(Stores population-specific network information.)doc";

static const char *__doc_bbp_sonata_PopulationProperties_alternateMorphologyFormats = R"doc(Dictionary for alternate directory paths.)doc";

static const char *__doc_bbp_sonata_PopulationProperties_biophysicalNeuronModelsDir = R"doc(Path to the template HOC files defining the E-Mode)doc";

static const char *__doc_bbp_sonata_PopulationProperties_morphologiesDir = R"doc(Path to the directory containing the morphologies)doc";

static const char *__doc_bbp_sonata_PopulationProperties_type = R"doc(Population type)doc";

static const char *__doc_bbp_sonata_PopulationStorage = R"doc(Collection of {PopulationClass}s stored in a H5 file and optional CSV.)doc";

static const char *__doc_bbp_sonata_PopulationStorage_Impl = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_2 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_3 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_impl = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_openPopulation =
R"doc(Open a specific {PopulationClass} by name

Parameter ``name``:
    the name of the population to open

Throws:
    if no population with such a name exists)doc";

static const char *__doc_bbp_sonata_PopulationStorage_populationNames = R"doc(Set of all {PopulationClass} names)doc";

static const char *__doc_bbp_sonata_Population_Impl = R"doc()doc";

static const char *__doc_bbp_sonata_Population_Population = R"doc()doc";

static const char *__doc_bbp_sonata_Population_Population_2 = R"doc()doc";

static const char *__doc_bbp_sonata_Population_Population_3 = R"doc()doc";

static const char *__doc_bbp_sonata_Population_attributeDataType =
R"doc(Get attribute data type, optionally translating enumeration types

\internal It is a helper method for dynamic languages bindings; and is
not intended for use in the ordinary client C++ code.)doc";

static const char *__doc_bbp_sonata_Population_attributeNames =
R"doc(All attribute names (CSV columns + required attributes + union of
attributes in groups))doc";

static const char *__doc_bbp_sonata_Population_dynamicsAttributeDataType =
R"doc(Get dynamics attribute data type

\internal It is a helper method for dynamic languages bindings; and is
not intended for use in the ordinary client C++ code.)doc";

static const char *__doc_bbp_sonata_Population_dynamicsAttributeNames =
R"doc(All dynamics attribute names (JSON keys + union of attributes in
groups))doc";

static const char *__doc_bbp_sonata_Population_enumerationNames =
R"doc(All attribute names that are explicit enumerations

See also: https://github.com/AllenInstitute/sonata/blob/master/docs/SO
NATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes)doc";

static const char *__doc_bbp_sonata_Population_enumerationValues =
R"doc(Get all allowed attribute enumeration values

Parameter ``name``:
    is a string to allow attributes not defined in spec

Throws:
    if there is no such attribute for the population)doc";

static const char *__doc_bbp_sonata_Population_filterAttribute = R"doc()doc";

static const char *__doc_bbp_sonata_Population_getAttribute =
R"doc(Get attribute values for given {element} Selection

If string values are requested and the attribute is a explicit
enumeration, values will be resolved to strings.

See also: https://github.com/AllenInstitute/sonata/blob/master/docs/SO
NATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes

Parameter ``name``:
    is a string to allow attributes not defined in spec

Parameter ``selection``:
    is a selection to retrieve the attribute values from

Throws:
    if there is no such attribute for the population

Throws:
    if the attribute is not defined for _any_ element from the
    selection)doc";

static const char *__doc_bbp_sonata_Population_getAttribute_2 =
R"doc(Get attribute values for given {element} Selection

If string values are requested and the attribute is a explicit
enumeration, values will be resolved to strings.

See also: https://github.com/AllenInstitute/sonata/blob/master/docs/SO
NATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes

Parameter ``name``:
    is a string to allow attributes not defined in spec

Parameter ``selection``:
    is a selection to retrieve the attribute values from

Parameter ``default``:
    is a value to use for {element}s without the given attribute

Throws:
    if there is no such attribute for the population)doc";

static const char *__doc_bbp_sonata_Population_getDynamicsAttribute =
R"doc(Get dynamics attribute values for given {element} Selection

Parameter ``name``:
    is a string to allow attributes not defined in spec

Parameter ``selection``:
    is a selection to retrieve the dynamics attribute values from

Throws:
    if there is no such attribute for the population

Throws:
    if the attribute is not defined for _any_ edge from the edge
    selection)doc";

static const char *__doc_bbp_sonata_Population_getDynamicsAttribute_2 =
R"doc(Get dynamics attribute values for given {element} Selection

Parameter ``name``:
    is a string to allow attributes not defined in spec

Parameter ``selection``:
    is a selection to retrieve the dynamics attribute values from

Parameter ``default``:
    is a value to use for {element}s without the given attribute

Throws:
    if there is no such attribute for the population)doc";

static const char *__doc_bbp_sonata_Population_getEnumeration =
R"doc(Get enumeration values for given attribute and {element} Selection

See also: https://github.com/AllenInstitute/sonata/blob/master/docs/SO
NATA_DEVELOPER_GUIDE.md#nodes---enum-datatypes

Parameter ``name``:
    is a string to allow enumeration attributes not defined in spec

Parameter ``selection``:
    is a selection to retrieve the enumeration values from

Throws:
    if there is no such attribute for the population

Throws:
    if the attribute is not defined for _any_ element from the
    selection)doc";

static const char *__doc_bbp_sonata_Population_impl = R"doc()doc";

static const char *__doc_bbp_sonata_Population_name = R"doc(Name of the population used for identifying it in circuit composition)doc";

static const char *__doc_bbp_sonata_Population_selectAll = R"doc(Selection covering all elements)doc";

static const char *__doc_bbp_sonata_Population_size = R"doc(Total number of elements)doc";

static const char *__doc_bbp_sonata_ReportReader = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_Population = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_data_units = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_get =
R"doc(Parameter ``node_ids``:
    limit the report to the given selection.

Parameter ``tstart``:
    return voltages occurring on or after tstart.
    tstart=nonstd::nullopt indicates no limit.

Parameter ``tstop``:
    return voltages occurring on or before tstop.
    tstop=nonstd::nullopt indicates no limit.

Parameter ``tstride``:
    indicates every how many timesteps we read data.
    tstride=nonstd::nullopt indicates that all timesteps are read.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getDataUnits = R"doc(Return the unit of data.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getIndex = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getNodeIdElementIdMapping =
R"doc(Return the ElementIds for the passed Node. The return type will depend
on the report reader: - For Soma report reader, the return value will
be the Node ID to which the report value belongs to. - For
Element/full compartment readers, the return value will be an array
with 2 elements, the first element is the Node ID and the second
element is the compartment ID of the given Node.

Parameter ``node_ids``:
    limit the report to the given selection. If nullptr, all nodes in
    the report are used

Parameter ``fn``:
    lambda applied to all ranges for all node ids)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getNodeIds = R"doc(Return all the node ids.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getSorted = R"doc(Return true if the data is sorted.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getTimeUnits = R"doc(Return the unit of time)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getTimes = R"doc(Return (tstart, tstop, tstep) of the population)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_node_ids_from_selection = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_nodes_ids = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_nodes_ids_sorted = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_nodes_pointers = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_pop_group = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_time_units = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_times_index = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_tstart = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_tstep = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_tstop = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_ReportReader = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_file = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_getPopulationNames = R"doc(Return a list of all population names.)doc";

static const char *__doc_bbp_sonata_ReportReader_openPopulation = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_populations = R"doc()doc";

static const char *__doc_bbp_sonata_Selection = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_Selection = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_Selection_2 = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_empty = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_flatSize = R"doc(Total number of elements constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_flatten = R"doc(Array of IDs constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_fromValues = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_fromValues_2 = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_ranges = R"doc(Get a list of ranges constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_ranges_2 = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig = R"doc(Read access to a SONATA simulation config file.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output = R"doc(Parameters to override simulator output for spike reports)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_outputDir = R"doc(Spike report file output directory. Default is "output")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_spikesFile = R"doc(Spike report file name. Default is "out.h5")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Parser = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report = R"doc(List of report parameters collected during the simulation)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_cells = R"doc(Node sets on which to report)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_dt = R"doc(Interval between reporting steps in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_endTime = R"doc(Time to stop reporting in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_fileName = R"doc(Report filename. Default is "<report name>_SONATA.h5")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_startTime = R"doc(Time to step reporting in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_type = R"doc(Report type. Possible values: "compartment", "summation", "synapse")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run = R"doc(Parameters defining global simulation settings for spike reports)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_dt = R"doc(Integration step duration in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_tstop = R"doc(Biological simulation end time in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulationConfig =
R"doc(Parses a SONATA JSON simulation configuration file.

Throws:
    SonataError on: - Ill-formed JSON - Missing mandatory entries (in
    any depth))doc";

static const char *__doc_bbp_sonata_SimulationConfig_basePath = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_fromFile =
R"doc(Loads a SONATA JSON simulation config file from disk and returns a
CircuitConfig object which parses it.

Throws:
    SonataError on: - Non accesible file (does not exists / does not
    have read access) - Ill-formed JSON - Missing mandatory entries
    (in any depth))doc";

static const char *__doc_bbp_sonata_SimulationConfig_getBasePath = R"doc(Returns the base path of the simulation config file)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getJSON = R"doc(Returns the JSON content of the simulation config file)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getOutput = R"doc(Returns the Output section of the simulation configuration.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getReport =
R"doc(Returns the given report parameters.

Throws:
    SonataError if the given report name does not correspond with any
    existing report.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getRun = R"doc(Returns the Run section of the simulation configuration.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_jsonContent = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_output = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_reports = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_run = R"doc()doc";

static const char *__doc_bbp_sonata_SonataError = R"doc()doc";

static const char *__doc_bbp_sonata_SonataError_SonataError = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader =
R"doc(const SpikeReader file(filename); auto pops =
file.getPopulationNames(); for (const auto& data:
file[pops.openPopulation(0).get(Selection{12UL, 34UL, 58UL})) { NodeID
node_id; double timestamp; std::tie(node_id, timestamp) = data;
std::cout << "[" << timestamp << "] " << node_id << std::endl; })doc";

static const char *__doc_bbp_sonata_SpikeReader_Population = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Population = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_by_id = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_by_time = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_none = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_filterNode = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_filterTimestamp = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_get = R"doc(Return reports for this population.)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getSorting = R"doc(Return the way data are sorted ('none', 'by_id', 'by_time'))doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getTimes = R"doc(Return (tstart, tstop) of the population)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_sorting = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_spikes = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_tstart = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_tstop = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_SpikeReader = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_filename = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_getPopulationNames = R"doc(Return a list of all population names.)doc";

static const char *__doc_bbp_sonata_SpikeReader_openPopulation = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_populations = R"doc()doc";

static const char *__doc_bbp_sonata_detail_NodeSets = R"doc()doc";

static const char *__doc_bbp_sonata_fromValues = R"doc()doc";

static const char *__doc_bbp_sonata_getAttribute = R"doc()doc";

static const char *__doc_bbp_sonata_operator_band = R"doc()doc";

static const char *__doc_bbp_sonata_operator_bor = R"doc()doc";

static const char *__doc_bbp_sonata_operator_eq = R"doc()doc";

static const char *__doc_bbp_sonata_operator_ne = R"doc()doc";

static const char *__doc_bbp_sonata_version = R"doc()doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

