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

static const char *__doc_bbp_sonata_CircuitConfig_Components_endfeetMeshesFile = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_microdomainsFile = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_morphologiesDir = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_spineMorphologiesDir = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_vasculatureFile = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_Components_vasculatureMesh = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_ConfigStatus = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_ConfigStatus_complete =
R"doc(all mandatory properties exist, and the config should return correct
values in all possible cases)doc";

static const char *__doc_bbp_sonata_CircuitConfig_ConfigStatus_invalid = R"doc(needed for parsing json contents that are null / not an enum value)doc";

static const char *__doc_bbp_sonata_CircuitConfig_ConfigStatus_partial =
R"doc(Partial configs relax the requirements: - can be missing the top level
networks key - can be missing the nodes and edges properties under
network - mandatory properties aren't enforced (for example for
biophysical circuits))doc";

static const char *__doc_bbp_sonata_CircuitConfig_Parser = R"doc()doc";

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

static const char *__doc_bbp_sonata_CircuitConfig_getCircuitConfigStatus =
R"doc(Returns the `completeness` of the checks that are performed for the
circuit; see `ConfigStatus` for more information)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getEdgePopulation =
R"doc(Creates and returns an EdgePopulation object, initialized from the
given population, and the edge network it belongs to.

Throws:
    SonataError if the given population does not exist in any edge
    network.)doc";

static const char *__doc_bbp_sonata_CircuitConfig_getEdgePopulation_2 = R"doc()doc";

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

static const char *__doc_bbp_sonata_CircuitConfig_getNodePopulation_2 = R"doc()doc";

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

static const char *__doc_bbp_sonata_CircuitConfig_nodePopulationProperties = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_nodeSetsFile = R"doc()doc";

static const char *__doc_bbp_sonata_CircuitConfig_status = R"doc()doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties = R"doc()doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_alternateMorphologyFormats = R"doc(Dictionary for alternate directory paths.)doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_biophysicalNeuronModelsDir = R"doc(Path to the template HOC files defining the E-Mode)doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_elementsPath =
R"doc(Path to underlying elements H5 file. It is discouraged to directly
access the contents of the file. Instead use 'libsonata' to read this
file.)doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_morphologiesDir = R"doc(Path to the directory containing the morphologies)doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_type = R"doc(Population type)doc";

static const char *__doc_bbp_sonata_CommonPopulationProperties_typesPath =
R"doc(Path to underlying types csv file. It is discouraged to directly
access the contents of the file. Instead use 'libsonata' to read this
file.)doc";

static const char *__doc_bbp_sonata_DataFrame = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_data = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_ids = R"doc()doc";

static const char *__doc_bbp_sonata_DataFrame_times = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulationProperties = R"doc(Edge population-specific network information.)doc";

static const char *__doc_bbp_sonata_EdgePopulationProperties_endfeetMeshesFile = R"doc(Path to spatial_segment_index)doc";

static const char *__doc_bbp_sonata_EdgePopulationProperties_spatialSynapseIndexDir = R"doc(Path to spatial_synapse_index)doc";

static const char *__doc_bbp_sonata_EdgePopulationProperties_spineMorphologiesDir = R"doc(Path to the directory containing the dendritic spine morphologies.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_EdgePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation_EdgePopulation_2 = R"doc()doc";

static const char *__doc_bbp_sonata_EdgePopulation_afferentEdges = R"doc(Return inbound edges for given node IDs.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_connectingEdges = R"doc(Return edges connecting two given nodes.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_efferentEdges = R"doc(Return outbound edges for given node IDs.)doc";

static const char *__doc_bbp_sonata_EdgePopulation_source = R"doc(Name of source population extracted from 'source_node_id' dataset)doc";

static const char *__doc_bbp_sonata_EdgePopulation_sourceNodeIDs = R"doc(Return source node IDs for a given edge selection)doc";

static const char *__doc_bbp_sonata_EdgePopulation_target = R"doc(Name of target population extracted from 'target_node_id' dataset)doc";

static const char *__doc_bbp_sonata_EdgePopulation_targetNodeIDs = R"doc(Return target node IDs for a given edge selection)doc";

static const char *__doc_bbp_sonata_EdgePopulation_writeIndices = R"doc(Write bidirectional node->edge indices to EdgePopulation HDF5.)doc";

static const char *__doc_bbp_sonata_Hdf5PluginInterface = R"doc()doc";

static const char *__doc_bbp_sonata_Hdf5PluginRead1DInterface = R"doc(Interface for implementing `readSelection<T>(dset, selection)`.)doc";

static const char *__doc_bbp_sonata_Hdf5PluginRead1DInterface_readSelection =
R"doc(Read the selected subset of the one-dimensional array.

The selection is canonical, i.e. sorted and non-overlapping. The
dataset is obtained from a `HighFive::File` opened via
`this->openFile`.)doc";

static const char *__doc_bbp_sonata_Hdf5PluginRead2DInterface = R"doc()doc";

static const char *__doc_bbp_sonata_Hdf5PluginRead2DInterface_readSelection =
R"doc(Read the Cartesian product of the two selections.

Both selections are canonical, i.e. sorted and non-overlapping. The
dataset is obtained from a `HighFive::File` opened via
`this->openFile`.)doc";

static const char *__doc_bbp_sonata_Hdf5Reader =
R"doc(Abstraction for reading HDF5 datasets.

The Hdf5Reader provides an interface for reading canonical selections
from datasets. Selections are canonical if they are sorted and don't
overlap. This allows implementing different optimization strategies,
such as minimizing bytes read, aggregating nearby reads or using MPI
collective I/O.

The design uses virtual inheritance, which enables users to inject
their own reader if needed. This class is the interface used within
libsonata. It simply delegates to a "plugin", that satisfies the
interface `Hdf5PluginInterface`.

To enable MPI collective I/O, `libsonata` must call all methods in an
MPI-collective manner. This implies that the number of times any
function in `libsonata` calls any of the `Hdf5Reader` methods must not
depend on the arguments to the function.

Examples:

void wrong(Selection selection) { // Wrong because some MPI ranks
might return without // calling `readSelection`. if(selection.empty())
{ return; } hdf5_reader.readSelection(dset, selection); }

void also_wrong(Selection selection) { // Wrong because `hdf5_reader`
is called `selection.ranges().size()` // number of times. Which could
be different on each MPI rank. for(auto range : selection.ranges()) {
hdf5_reader.readSelection(dset, Selection(std::vector{range})); } }

void subtle(Selection selection, bool flag) { // If the flag can
differ between MPI ranks, this is wrong because // `readSelection` is
called with different `dset`s. If the `flag` must // be the same on
all MPI ranks, this is correct. If this happens in // the libsonata
API, then passing the same `flag` on all MPI ranks becomes // a
requirement for the users, when using a collective reader. Example: //
pop.get_attribute(attr_name, selection) if(flag) {
hdf5_reader.readSelection(dset1, selection); } else {
hdf5_reader.readSelection(dset2, selection); } }

void correct(Selection selection) { // Correct because no matter which
branch is taken // `hdf5_reader.readSelection` is called exactly once.
if(selection.size % 2 == 0) { hdf5_reader.readSelection(dset,
selection); } else { hdf5_reader.readSelection(dset, {}); } })doc";

static const char *__doc_bbp_sonata_Hdf5Reader_Hdf5Reader = R"doc(Create a valid Hdf5Reader with the default plugin.)doc";

static const char *__doc_bbp_sonata_Hdf5Reader_Hdf5Reader_2 = R"doc(Create an Hdf5Reader with a user supplied plugin.)doc";

static const char *__doc_bbp_sonata_Hdf5Reader_impl = R"doc()doc";

static const char *__doc_bbp_sonata_Hdf5Reader_openFile =
R"doc(Open the HDF5.

The dataset passed to `readSelection` must be obtained from a file
open via this method.)doc";

static const char *__doc_bbp_sonata_Hdf5Reader_readSelection =
R"doc(Read the selected subset of the one-dimensional array.

Both selections are canonical, i.e. sorted and non-overlapping. The
dataset is obtained from a `HighFive::File` opened via
`this->openFile`.)doc";

static const char *__doc_bbp_sonata_Hdf5Reader_readSelection_2 =
R"doc(Read the Cartesian product of the two selections.

Both selections are canonical, i.e. sorted and non-overlapping. The
dataset is obtained from a `HighFive::File` opened via
`this->openFile`.)doc";

static const char *__doc_bbp_sonata_NodePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_NodePopulationProperties = R"doc(Node population-specific network information.)doc";

static const char *__doc_bbp_sonata_NodePopulationProperties_microdomainsFile =
R"doc(Path to the .h5 storing microdomain data. Only for astrocyte node
populations where it is mandatory.)doc";

static const char *__doc_bbp_sonata_NodePopulationProperties_spatialSegmentIndexDir = R"doc(Path to spatial_segment_index)doc";

static const char *__doc_bbp_sonata_NodePopulationProperties_vasculatureFile =
R"doc(Path to the .h5 file containing the vasculature morphology. Only for
vasculature node populations where it is mandatory.)doc";

static const char *__doc_bbp_sonata_NodePopulationProperties_vasculatureMesh =
R"doc(Path to the .obj file containing the mesh of a vasculature morphology.
Only for vasculature node populations where it is mandatory.)doc";

static const char *__doc_bbp_sonata_NodePopulation_NodePopulation = R"doc()doc";

static const char *__doc_bbp_sonata_NodePopulation_NodePopulation_2 = R"doc()doc";

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

static const char *__doc_bbp_sonata_NodeSets_NodeSets_4 = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_fromFile = R"doc(Open a SONATA `node sets` file from a path */)doc";

static const char *__doc_bbp_sonata_NodeSets_impl = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_materialize =
R"doc(Return a selection corresponding to the node_set name

Parameter ``name``:
    is the name of the node_set rule to be evaluated

Parameter ``population``:
    is the population for which the returned selection will be valid)doc";

static const char *__doc_bbp_sonata_NodeSets_names = R"doc(Names of the node sets available)doc";

static const char *__doc_bbp_sonata_NodeSets_operator_assign = R"doc()doc";

static const char *__doc_bbp_sonata_NodeSets_toJSON = R"doc(Return the nodesets as a JSON string.)doc";

static const char *__doc_bbp_sonata_NodeSets_update =
R"doc(Update `this` to include all nodesets from `this` and `other`.

Duplicate names are overridden with the values from `other`.

The duplicate names are returned.)doc";

static const char *__doc_bbp_sonata_Population = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage = R"doc(Collection of {PopulationClass}s stored in a H5 file and optional CSV.)doc";

static const char *__doc_bbp_sonata_PopulationStorage_Impl = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_2 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_3 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_4 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_5 = R"doc()doc";

static const char *__doc_bbp_sonata_PopulationStorage_PopulationStorage_6 = R"doc()doc";

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

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout_ids = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout_min_max_blocks = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout_node_index = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout_node_offsets = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_NodeIdElementLayout_node_ranges = R"doc()doc";

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
    tstride=nonstd::nullopt indicates that all timesteps are read.

Parameter ``block_gap_limit``:
    gap limit between each IO block while fetching data from storage.)doc";

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

Parameter ``block_gap_limit``:
    gap limit between each IO block while fetching data from storage)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getNodeIdElementLayout =
R"doc(Return the element IDs for the given selection, alongside the filtered
node pointers and the range of positions where they fit in the file.
This latter two are necessary for performance to understand how and
where to retrieve the data from storage.

Parameter ``node_ids``:
    limit the report to the given selection. If nullptr, all nodes in
    the report are used

Parameter ``block_gap_limit``:
    gap limit between each IO block while fetching data from storage)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getNodeIds = R"doc(Return all the node ids.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getSorted = R"doc(Return true if the data is sorted.)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getTimeUnits = R"doc(Return the unit of time)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_getTimes = R"doc(Return (tstart, tstop, tstep) of the population)doc";

static const char *__doc_bbp_sonata_ReportReader_Population_is_node_ids_sorted = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_node_ids = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_node_index = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_node_offsets = R"doc()doc";

static const char *__doc_bbp_sonata_ReportReader_Population_node_ranges = R"doc()doc";

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

static const char *__doc_bbp_sonata_Selection_empty = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_flatSize = R"doc(Total number of elements constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_flatten = R"doc(Array of IDs constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_fromValues = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_fromValues_2 = R"doc()doc";

static const char *__doc_bbp_sonata_Selection_ranges = R"doc(Get a list of ranges constituting Selection)doc";

static const char *__doc_bbp_sonata_Selection_ranges_2 = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig = R"doc(Read access to a SONATA simulation config file.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions = R"doc(Parameters defining global experimental conditions.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_SpikeLocation = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_SpikeLocation_AIS = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_SpikeLocation_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_SpikeLocation_soma = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_celsius = R"doc(Temperature of experiment. Default is 34.0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_extracellularCalcium =
R"doc(Extracellular calcium concentration, being applied to the synapse
uHill parameter in order to scale the U parameter of synapses. Default
is None.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_getModifications =
R"doc(Method to return the full list of modifications in the Conditions
section.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_mechanisms =
R"doc(Properties to assign values to variables in synapse MOD files. The
format is a dictionary with keys being the SUFFIX names and values
being dictionaries of variables' names and values.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_modifications =
R"doc(List of modifications that mimics experimental manipulations to the
circuit.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_randomizeGabaRiseTime =
R"doc(Enable legacy behavior to randomize the GABA_A rise time in the helper
functions. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_spikeLocation =
R"doc(The spike detection location. Can be either ‘soma’ or 'AIS'. Default
is 'soma')doc";

static const char *__doc_bbp_sonata_SimulationConfig_Conditions_vInit = R"doc(Initial membrane voltage in mV. Default is -80)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride =
R"doc(List of connection parameters to adjust the synaptic strength or other
properties of edges between two sets of nodes)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_delay =
R"doc(Adjustments from weight of this connection_override are applied after
the specified delay has elapsed in ms, default = 0.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_modoverride =
R"doc(Synapse helper files to instantiate the synapses in this
connection_override, default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_name = R"doc(the name of the connection override)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_neuromodulationDtc =
R"doc(To override the neuromod_dtc values between the selected source and
target neurons for the neuromodulatory projection. Given in ms.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_neuromodulationStrength =
R"doc(To override the neuromod_strength values between the selected source
and target neurons for the neuromodulatory projection. Given in muM.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_source = R"doc(node_set specifying presynaptic nodes)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_spontMinis =
R"doc(Rate to spontaneously trigger the synapses in this
connection_override, default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_synapseConfigure =
R"doc(Snippet of hoc code to be executed on the synapses in this
connection_override, default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_synapseDelayOverride =
R"doc(Value to override the synaptic delay time originally set in the edge
file (ms), default = None.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_target = R"doc(node_set specifying postsynaptic nodes)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ConnectionOverride_weight = R"doc(Scalar to adjust synaptic strength, default = 1.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_decayTime = R"doc(The decay time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_dt = R"doc(Timestep of generated signal in ms. Default is 0.25 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_mean = R"doc(Signal mean in nA (current_clamp) or uS (conductance).)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_randomSeed =
R"doc(Override the random seed to introduce correlations between cells,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_relativeSkew =
R"doc(Signal skewness as a fraction in [0, 1] representing a value between
the minimum and maximum skewness values compatible with the given
signal mean and std dev. Default is 0.5.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_reversal = R"doc(Reversal potential for conductance injection in mV. Default is 0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_riseTime = R"doc(The rise time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputAbsoluteShotNoise_sigma = R"doc(signal std dev in nA (current_clamp) or uS (conductance).)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_conductance = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_current_clamp = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_extracellular_stimulation = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_spikes = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_InputType_voltage_clamp = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_absolute_shot_noise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_hyperpolarizing = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_linear = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_noise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_ornstein_uhlenbeck = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_pulse = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_relative_linear = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_relative_ornstein_uhlenbeck = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_relative_shot_noise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_seclamp = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_shot_noise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_sinusoidal = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_subthreshold = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_Module_synapse_replay = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_delay = R"doc(Time when input is activated (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_duration = R"doc(Time duration for how long input is activated (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_inputType = R"doc(Type of input)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_module = R"doc(Type of stimulus)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputBase_nodeSet = R"doc(Node set which is affected by input)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputHyperpolarizing = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputHyperpolarizing_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputLinear = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputLinear_ampEnd = R"doc(The final current when a stimulus concludes (nA))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputLinear_ampStart = R"doc(The amount of current initially injected (nA))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputLinear_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputNoise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputNoise_mean = R"doc(The mean value of current to inject (nA), default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputNoise_meanPercent =
R"doc(The mean value of current to inject as a percentage of threshold
current, default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputNoise_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputNoise_variance =
R"doc(State var to track whether the value of injected noise current is mean
or mean_percent)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_dt = R"doc(Timestep of generated signal in ms. Default is 0.25 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_mean = R"doc(Signal mean in nA (current_clamp) or uS (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_randomSeed =
R"doc(Override the random seed to introduce correlations between cells,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_reversal = R"doc(Reversal potential for conductance injection in mV. Default is 0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_sigma = R"doc(Signal std dev in nA (current_clamp) or uS (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputOrnsteinUhlenbeck_tau = R"doc(Relaxation time constant in ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputPulse = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputPulse_ampStart = R"doc(The amount of current initially injected (nA))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputPulse_frequency = R"doc(The frequency of pulse trains (Hz))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputPulse_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputPulse_width = R"doc(The length of time each pulse lasts (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeLinear = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeLinear_percentEnd = R"doc(The percentage of a cell's threshold current to inject at the end)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeLinear_percentStart = R"doc(The percentage of a cell's threshold current to inject)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeLinear_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_dt = R"doc(Timestep of generated signal in ms. Default is 0.25 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_meanPercent =
R"doc(Signal mean as percentage of a cell’s threshold current
(current_clamp) or inverse input resistance (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_randomSeed =
R"doc(Override the random seed to introduce correlations between cells,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_reversal = R"doc(Reversal potential for conductance injection in mV. Default is 0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_sdPercent =
R"doc(Signal std dev as percentage of a cell’s threshold current
(current_clamp) or inverse input resistance (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeOrnsteinUhlenbeck_tau = R"doc(Relaxation time constant in ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_decayTime = R"doc(The decay time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_dt = R"doc(Timestep of generated signal in ms. Default is 0.25 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_meanPercent =
R"doc(Signal mean as percentage of a cell’s threshold current
(current_clamp) or inverse input resistance (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_randomSeed =
R"doc(Override the random seed to introduce correlations between cells,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_relativeSkew =
R"doc(Signal skewness as a fraction in [0, 1] representing a value between
the minimum and maximum skewness values compatible with the given
signal mean and std dev. Default is 0.5.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_reversal = R"doc(Reversal potential for conductance injection in mV. Default is 0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_riseTime = R"doc(The rise time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputRelativeShotNoise_sdPercent =
R"doc(signal std dev as percentage of a cell’s threshold current
(current_clamp) or inverse input resistance (conductance).)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSeclamp = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSeclamp_seriesResistance = R"doc(The series resistance (Mohm), default is 0.01 Mohm)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSeclamp_voltage = R"doc(The membrane voltage the targeted cells should be held at (mV))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_ampMean =
R"doc(The mean of gamma-distributed amplitudes in nA (current_clamp) or uS
(conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_ampVar =
R"doc(The variance of gamma-distributed amplitudes in nA^2 (current_clamp)
or uS^2 (conductance))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_decayTime = R"doc(The decay time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_dt = R"doc(Timestep of generated signal in ms. Default is 0.25 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_randomSeed =
R"doc(Override the random seed to introduce correlations between cells,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_rate = R"doc(Rate of Poisson events (Hz))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_reversal = R"doc(Reversal potential for conductance injection in mV. Default is 0)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputShotNoise_riseTime = R"doc(The rise time of the bi-exponential shots (ms))doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSinusoidal = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSinusoidal_ampStart = R"doc(The peak amplitude of the sinusoid. Given in nA.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSinusoidal_dt = R"doc(Timestep of generated signal in ms. Default is 0.025 ms)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSinusoidal_frequency = R"doc(The frequency of the sinusoidal waveform. Given in Hz.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSinusoidal_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSubthreshold = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSubthreshold_percentLess = R"doc(A percentage adjusted from 100 of a cell's threshold current)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSubthreshold_representsPhysicalElectrode = R"doc(Whether this input represents a physical electrode. Default is false)doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSynapseReplay = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_InputSynapseReplay_spikeFile =
R"doc(The location of the file with the spike info for injection, file
extension must be .h5)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_ModificationType = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_ModificationType_ConfigureAllSections = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_ModificationType_TTX = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_ModificationType_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_name = R"doc(Name of the modification setting.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_nodeSet = R"doc(Node set which receives the manipulation)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationBase_type =
R"doc(Name of the manipulation. Supported values are “TTX” and
“ConfigureAllSections”.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationConfigureAllSections = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationConfigureAllSections_sectionConfigure =
R"doc(For “ConfigureAllSections” manipulation, a snippet of python code to
perform one or more assignments involving section attributes, for all
sections that have all the referenced attributes. The format is
"%s.xxxx; %s.xxxx; ...".)doc";

static const char *__doc_bbp_sonata_SimulationConfig_ModificationTTX = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output = R"doc(Parameters to override simulator output for spike reports)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_SpikesSortOrder = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_SpikesSortOrder_by_id = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_SpikesSortOrder_by_time = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_SpikesSortOrder_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_SpikesSortOrder_none = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_logFile = R"doc(Filename where console output is written. Default is STDOUT.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_outputDir = R"doc(Spike report file output directory. Default is "output")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_sortOrder = R"doc(The sorting order of the spike report. Default is "by_time")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Output_spikesFile = R"doc(Spike report file name. Default is "out.h5")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Parser = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report = R"doc(List of report parameters collected during the simulation)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Compartments = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Compartments_all = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Compartments_center = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Compartments_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Scaling = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Scaling_area = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Scaling_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Scaling_none = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_all = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_apic = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_axon = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_dend = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Sections_soma = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type_compartment = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type_lfp = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type_summation = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_Type_synapse = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_cells = R"doc(Node sets on which to report)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_compartments =
R"doc(For compartment type, select compartments to report. Default value:
"center"(for sections: soma), "all"(for other sections))doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_dt = R"doc(Interval between reporting steps in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_enabled =
R"doc(Allows for suppressing a report so that is not created. Default is
true)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_endTime = R"doc(Time to stop reporting in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_fileName = R"doc(Report filename. Default is "<report name>.h5")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_scaling =
R"doc(For summation type, specify the handling of density values. Default
value: "area")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_sections = R"doc(Sections on which to report. Default value: "soma")doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_startTime = R"doc(Time to step reporting in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_type = R"doc(Report type.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_unit = R"doc(Descriptive text of the unit recorded. Not validated for correctness)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Report_variableName =
R"doc(The simulation variable to access. The variables available are model
dependent. For summation type, it supports multiple variables by comma
separated strings. E.g. “ina”, "AdEx.V_M, v", "i_membrane, IClamp".)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run = R"doc(Parameters defining global simulation settings for spike reports)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_IntegrationMethod = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_IntegrationMethod_crank_nicolson = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_IntegrationMethod_crank_nicolson_ion = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_IntegrationMethod_euler = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_IntegrationMethod_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_dt = R"doc(Integration step duration in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_electrodesFile = R"doc(Filename that contains the weights for the LFP calculation.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_integrationMethod =
R"doc(Selects the NEURON/CoreNEURON integration method. This parameter sets
the NEURON global variable h.secondorder, default is "euler".)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_ionchannelSeed =
R"doc(A non-negative integer used for seeding stochastic ion channels,
default is 0.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_minisSeed =
R"doc(A non-negative integer used for seeding the Poisson processes that
drives the minis, default is 0.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_randomSeed = R"doc(Random seed)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_spikeThreshold = R"doc(The spike detection threshold. Default is -30mV)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_stimulusSeed =
R"doc(A non-negative integer used for seeding noise stimuli and any other
future stochastic stimuli, default is 0.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_synapseSeed =
R"doc(A non-negative integer used for seeding stochastic synapses, default
is 0.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_Run_tstop = R"doc(Biological simulation end time in milliseconds)doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulationConfig =
R"doc(Parses a SONATA JSON simulation configuration file.

Throws:
    SonataError on: - Ill-formed JSON - Missing mandatory entries (in
    any depth))doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulatorType = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulatorType_CORENEURON = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulatorType_NEURON = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_SimulatorType_invalid = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_basePath = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_betaFeatures = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_conditions = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_connection_overrides = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_expandedJSON = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_fromFile =
R"doc(Loads a SONATA JSON simulation config file from disk and returns a
SimulationConfig object which parses it.

Throws:
    SonataError on: - Non accesible file (does not exists / does not
    have read access) - Ill-formed JSON - Missing mandatory entries
    (in any depth))doc";

static const char *__doc_bbp_sonata_SimulationConfig_getBasePath = R"doc(Returns the base path of the simulation config file)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getBetaFeatures = R"doc(Returns the beta_features section)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getConditions = R"doc(Returns the Conditions section of the simulation configuration.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getConnectionOverrides = R"doc(Returns the full list of connection overrides)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getExpandedJSON =
R"doc(Returns the configuration file JSON whose variables have been expanded
by the manifest entries.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getInput =
R"doc(Returns the given input parameters.

Throws:
    SonataError if the given input name does not exist)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getJSON = R"doc(Returns the JSON content of the simulation config file)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getMetaData = R"doc(Returns the metadata section)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getNetwork = R"doc(Returns circuit config file associated with this simulation config)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getNodeSet =
R"doc(Returns the name of node set to be instantiated for the simulation,
default = None)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getNodeSetsFile =
R"doc(Returns the path of node sets file overriding node_sets_file provided
in _network, default is empty in case of no setting in _network)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getOutput = R"doc(Returns the Output section of the simulation configuration.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getReport =
R"doc(Returns the given report parameters.

Throws:
    SonataError if the given report name does not correspond with any
    existing report.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getRun = R"doc(Returns the Run section of the simulation configuration.)doc";

static const char *__doc_bbp_sonata_SimulationConfig_getTargetSimulator =
R"doc(Returns the name of simulator, default = NEURON

Throws:
    SonataError if the given value is neither NEURON nor CORENEURON)doc";

static const char *__doc_bbp_sonata_SimulationConfig_inputs = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_listInputNames = R"doc(Returns the names of the inputs)doc";

static const char *__doc_bbp_sonata_SimulationConfig_listReportNames = R"doc(Returns the names of the reports)doc";

static const char *__doc_bbp_sonata_SimulationConfig_metaData = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_network = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_nodeSet = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_nodeSetsFile = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_output = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_reports = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_run = R"doc()doc";

static const char *__doc_bbp_sonata_SimulationConfig_targetSimulator = R"doc()doc";

static const char *__doc_bbp_sonata_SonataError = R"doc()doc";

static const char *__doc_bbp_sonata_SonataError_SonataError = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader = R"doc(Used to read spike files)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Population = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_by_id = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_by_time = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_Sorting_none = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_createSpikes = R"doc(Create the spikes from the vectors of node_ids and timestamps)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_filterNode = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_filterTimestamp = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_get = R"doc(Return spikes with all those node_ids between 'tstart' and 'tstop')doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getArrays =
R"doc(Return the node_ids and timestamps vectors with all node_ids between
'tstart' and 'tstop')doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getRawArrays = R"doc(Return the raw node_ids and timestamps vectors)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getSorting = R"doc(Return the way data are sorted ('none', 'by_id', 'by_time'))doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getTimeUnits = R"doc(Return the unit of time)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_getTimes = R"doc(Return (tstart, tstop) of the population)doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_sorting = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_spike_times = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_time_units = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_tstart = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_Population_tstop = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_SpikeReader = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_filename = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_getPopulationNames = R"doc(Return a list of all population names.)doc";

static const char *__doc_bbp_sonata_SpikeReader_openPopulation = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeReader_populations = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeTimes = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeTimes_node_ids = R"doc()doc";

static const char *__doc_bbp_sonata_SpikeTimes_timestamps = R"doc()doc";

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

