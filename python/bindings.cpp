#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <bbp/sonata/common.h>
#include <bbp/sonata/config.h>
#include <bbp/sonata/edges.h>
#include <bbp/sonata/node_sets.h>
#include <bbp/sonata/nodes.h>
#include <bbp/sonata/report_reader.h>

#include "generated/docstrings.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstdint>
#include <memory>
#include <string>

namespace py = pybind11;

using namespace pybind11::literals;

using namespace bbp::sonata;


namespace {

template <typename T>
py::capsule freeWhenDone(T* ptr) {
    return py::capsule(ptr, [](void* p) { delete reinterpret_cast<T*>(p); });
}


// This little beauty passes std::vector data to NumPy array without extra copy.
// For the details, please refer to:
// https://github.com/pybind/pybind11/issues/1042#issuecomment-325941022
template <typename T>
py::array asArray(std::vector<T>&& values) {
    auto ptr = new std::vector<T>(std::move(values));
    return py::array(ptr->size(), ptr->data(), freeWhenDone(ptr));
}


// As std::string is not a POD type, we wrap each value with py::object
template <>
py::array asArray(std::vector<std::string>&& values) {
    auto ptr = new std::vector<py::object>();
    ptr->reserve(values.size());
    for (auto& s : values) {
        ptr->push_back(py::cast(s));
    }
    return py::array(py::dtype("object"), ptr->size(), ptr->data(), freeWhenDone(ptr));
}


// Return a new Numpy array with data owned by another python object
// This avoids copies, and enables correct reference counting for memory keep-alive
template <typename DATA_T, typename DIMS_T, typename OWNER_T>
py::array managedMemoryArray(const DATA_T* data, const DIMS_T& dims, const OWNER_T& owner) {
    const auto& tinfo = py::detail::get_type_info(typeid(OWNER_T));
    const auto& handle = py::detail::get_object_handle(&owner, tinfo);
    return py::array(dims, data, handle);
}


template <typename T>
py::object getAttribute(const Population& obj,
                        const std::string& name,
                        const Selection& selection) {
    return py::cast(obj.getAttribute<T>(name, selection)[0]);
}


template <typename T>
py::object getAttributeVector(const Population& obj,
                              const std::string& name,
                              const Selection& selection) {
    return asArray(obj.getAttribute<T>(name, selection));
}


template <typename T>
py::object getEnumerationVector(const Population& obj,
                                const std::string& name,
                                const Selection& selection) {
    return asArray(obj.getEnumeration<T>(name, selection));
}


template <typename T>
py::object getAttributeVectorWithDefault(const Population& obj,
                                         const std::string& name,
                                         const Selection& selection,
                                         const py::object& defaultValue) {
    return asArray(obj.getAttribute<T>(name, selection, defaultValue.cast<T>()));
}


template <typename T>
py::object getDynamicsAttribute(const Population& obj,
                                const std::string& name,
                                const Selection& selection) {
    return py::cast(obj.getDynamicsAttribute<T>(name, selection)[0]);
}


template <typename T>
py::object getDynamicsAttributeVector(const Population& obj,
                                      const std::string& name,
                                      const Selection& selection) {
    return asArray(obj.getDynamicsAttribute<T>(name, selection));
}


template <typename T>
py::object getDynamicsAttributeVectorWithDefault(const Population& obj,
                                                 const std::string& name,
                                                 const Selection& selection,
                                                 const py::object& defaultValue) {
    return asArray(obj.getDynamicsAttribute<T>(name, selection, defaultValue.cast<T>()));
}

// create a macro to reduce repetition for docstrings
#define DOC_NODESETS(x) DOC(bbp, sonata, NodeSets, x)
#define DOC_SEL(x) DOC(bbp, sonata, Selection, x)
#define DOC_POP(x) DOC(bbp, sonata, Population, x)
#define DOC_POP_NODE(x) DOC(bbp, sonata, NodePopulation, x)
#define DOC_POP_EDGE(x) DOC(bbp, sonata, EdgePopulation, x)
#define DOC_POP_STOR(x) DOC(bbp, sonata, PopulationStorage, x)
#define DOC_SPIKEREADER_POP(x) DOC(bbp, sonata, SpikeReader, Population, x)
#define DOC_SPIKEREADER(x) DOC(bbp, sonata, SpikeReader, x)
#define DOC_REPORTREADER_POP(x) DOC(bbp, sonata, ReportReader, Population, x)

// Emulating generic lambdas in pre-C++14
#define DISPATCH_TYPE(dtype, func, ...)                               \
    if (dtype == "int8_t") {                                          \
        return func<int8_t>(__VA_ARGS__);                             \
    } else if (dtype == "uint8_t") {                                  \
        return func<uint8_t>(__VA_ARGS__);                            \
    } else if (dtype == "int16_t") {                                  \
        return func<int16_t>(__VA_ARGS__);                            \
    } else if (dtype == "uint16_t") {                                 \
        return func<uint16_t>(__VA_ARGS__);                           \
    } else if (dtype == "int32_t") {                                  \
        return func<int32_t>(__VA_ARGS__);                            \
    } else if (dtype == "uint32_t") {                                 \
        return func<uint32_t>(__VA_ARGS__);                           \
    } else if (dtype == "int64_t") {                                  \
        return func<int64_t>(__VA_ARGS__);                            \
    } else if (dtype == "uint64_t") {                                 \
        return func<uint64_t>(__VA_ARGS__);                           \
    } else if (dtype == "float") {                                    \
        return func<float>(__VA_ARGS__);                              \
    } else if (dtype == "double") {                                   \
        return func<double>(__VA_ARGS__);                             \
    } else if (dtype == "string") {                                   \
        return func<std::string>(__VA_ARGS__);                        \
    } else {                                                          \
        throw SonataError(std::string("Unexpected dtype: ") + dtype); \
    }


template <typename Population>
py::class_<Population, std::shared_ptr<Population>> bindPopulationClass(py::module& m,
                                                                        const char* clsName,
                                                                        const char* docString) {
    m.attr("version") = version();

    const auto imbueElementName = [](const char* msg) {
        return fmt::format(msg, fmt::arg("element", Population::ELEMENT));
    };
    return py::class_<Population, std::shared_ptr<Population>>(m, clsName, docString)
        .def(py::init<const std::string&, const std::string&, const std::string&>())
        .def_property_readonly("name", &Population::name, DOC_POP(name))
        .def_property_readonly("size", &Population::size, imbueElementName(DOC_POP(size)).c_str())
        .def_property_readonly("attribute_names",
                               &Population::attributeNames,
                               DOC_POP(attributeNames))
        .def_property_readonly("enumeration_names",
                               &Population::enumerationNames,
                               DOC_POP(enumerationNames))
        .def("__len__", &Population::size, imbueElementName(DOC_POP(size)).c_str())
        .def("__repr__",
             [clsName](Population& obj) {
                 return fmt::format("{} [name={}, count={}]", clsName, obj.name(), obj.size());
             })
        .def("select_all", &Population::selectAll, imbueElementName(DOC_POP(selectAll)).c_str())
        .def("enumeration_values",
             &Population::enumerationValues,
             py::arg("name"),
             DOC_POP(enumerationValues))
        .def(
            "get_attribute",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._attributeDataType(name, true);
                DISPATCH_TYPE(dtype, getAttribute, obj, name, selection);
            },
            py::arg("name"),
            py::arg(imbueElementName("{element}_id").c_str()),
            imbueElementName("Get attribute value for a given {element}.\n"
                             "See below for details.")
                .c_str())
        .def(
            "get_attribute",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._attributeDataType(name, true);
                DISPATCH_TYPE(dtype, getAttributeVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName(DOC_POP(getAttribute)).c_str())
        .def(
            "get_attribute",
            [](Population& obj,
               const std::string& name,
               const Selection& selection,
               const py::object& defaultValue) {
                const auto dtype = obj._attributeDataType(name, true);
                DISPATCH_TYPE(
                    dtype, getAttributeVectorWithDefault, obj, name, selection, defaultValue);
            },
            "name"_a,
            "selection"_a,
            "default_value"_a,
            imbueElementName(DOC_POP(getAttribute)).c_str())
        .def_property_readonly("dynamics_attribute_names",
                               &Population::dynamicsAttributeNames,
                               DOC_POP(dynamicsAttributeNames))
        .def(
            "get_dynamics_attribute",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._dynamicsAttributeDataType(name);
                DISPATCH_TYPE(dtype, getDynamicsAttribute, obj, name, selection);
            },
            py::arg("name"),
            py::arg(imbueElementName("{element}_id").c_str()),
            imbueElementName("Get dynamics attribute value for a given {element}.\n"
                             "See below for details.")
                .c_str())
        .def(
            "get_dynamics_attribute",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._dynamicsAttributeDataType(name);
                DISPATCH_TYPE(dtype, getDynamicsAttributeVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName(DOC_POP(getDynamicsAttribute)).c_str())
        .def(
            "get_dynamics_attribute",
            [](Population& obj,
               const std::string& name,
               const Selection& selection,
               const py::object& defaultValue) {
                const auto dtype = obj._dynamicsAttributeDataType(name);
                DISPATCH_TYPE(dtype,
                              getDynamicsAttributeVectorWithDefault,
                              obj,
                              name,
                              selection,
                              defaultValue);
            },
            "name"_a,
            "selection"_a,
            "default_value"_a,
            imbueElementName(DOC_POP(getDynamicsAttribute)).c_str())
        .def(
            "get_enumeration",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getEnumerationVector, obj, name, selection);
            },
            "name"_a,
            py::arg(imbueElementName("{element}_id").c_str()),
            imbueElementName("Get enumeration values for a given {element}.\n"
                             "See below for details.")
                .c_str())
        .def(
            "get_enumeration",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getEnumerationVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName(DOC_POP(enumerationValues)).c_str());
}


template <typename Storage>
py::class_<Storage> bindStorageClass(py::module& m, const char* clsName, const char* popClsName) {
    const auto imbuePopulationClassName = [popClsName](const char* msg) {
        return fmt::format(msg, fmt::arg("PopulationClass", popClsName));
    };
    return py::class_<Storage>(
               m, clsName, imbuePopulationClassName(DOC(bbp, sonata, PopulationStorage)).c_str())
        .def(py::init([](py::object h5_filepath, py::object csv_filepath) {
                 return Storage(py::str(h5_filepath), py::str(csv_filepath));
             }),
             "h5_filepath"_a,
             "csv_filepath"_a = "")
        .def_property_readonly("population_names",
                               &Storage::populationNames,
                               imbuePopulationClassName(DOC_POP_STOR(populationNames)).c_str())
        .def("open_population",
             &Storage::openPopulation,
             "name"_a,
             imbuePopulationClassName(DOC_POP_STOR(openPopulation)).c_str());
}
}  // unnamed namespace

namespace pybind11 {
namespace detail {
template <typename T>
struct type_caster<nonstd::optional<T>>: optional_caster<nonstd::optional<T>> {};

template <>
struct type_caster<nonstd::nullopt_t>: public void_caster<nonstd::nullopt_t> {};
}  // namespace detail
}  // namespace pybind11


template <typename ReportType, typename KeyType>
void bindReportReader(py::module& m, const std::string& prefix) {
    py::class_<DataFrame<KeyType>>(m,
                                   (prefix + "DataFrame").c_str(),
                                   "A container of raw reporting data, compatible with Pandas")
        // .ids, .data and .time members are owned by the c++ object. We can't do std::move.
        // To avoid copies, we must declare the owner of the data as the current Python
        // object. Numpy will adjust owner reference count according to returned arrays
        // clang-format off
        .def_property_readonly("ids", [](const DataFrame<KeyType>& dframe) {
            std::array<ssize_t, 1> dims { ssize_t(dframe.ids.size()) };
            return managedMemoryArray(dframe.ids.data(), dims, dframe);
        })
        .def_property_readonly("data", [](const DataFrame<KeyType>& dframe) {
            std::array<ssize_t, 2> dims {0l, ssize_t(dframe.ids.size())};
            if (dims[1] > 0) {
                dims[0] = dframe.data.size() / dims[1];
            }
            return managedMemoryArray(dframe.data.data(), dims, dframe);
        })
        // clang-format on
        .def_property_readonly("times", [](DataFrame<KeyType>& dframe) {
            return managedMemoryArray(dframe.times.data(), dframe.times.size(), dframe);
        });

    py::class_<typename ReportType::Population>(m,
                                                (prefix + "ReportPopulation").c_str(),
                                                "A population inside a ReportReader")
        .def("get",
             &ReportType::Population::get,
             "Return reports with all those node_ids between 'tstart' and 'tstop' with a stride "
             "tstride",
             "node_ids"_a = nonstd::nullopt,
             "tstart"_a = nonstd::nullopt,
             "tstop"_a = nonstd::nullopt,
             "tstride"_a = nonstd::nullopt)
        .def("get_node_ids",
             &ReportType::Population::getNodeIds,
             "Return the list of nodes ids for this population")
        .def(
            "get_node_id_element_id_mapping",
            [](const typename ReportType::Population& population,
               const nonstd::optional<Selection>& selection) {
                return population.getNodeIdElementIdMapping(selection);
            },
            DOC_REPORTREADER_POP(getNodeIdElementIdMapping),
            "selection"_a = nonstd::nullopt)
        .def_property_readonly("sorted",
                               &ReportType::Population::getSorted,
                               DOC_REPORTREADER_POP(getSorted))
        .def_property_readonly("times",
                               &ReportType::Population::getTimes,
                               DOC_REPORTREADER_POP(getTimes))
        .def_property_readonly("time_units",
                               &ReportType::Population::getTimeUnits,
                               DOC_REPORTREADER_POP(getTimeUnits))
        .def_property_readonly("data_units",
                               &ReportType::Population::getDataUnits,
                               DOC_REPORTREADER_POP(getDataUnits));
    py::class_<ReportType>(m, (prefix + "ReportReader").c_str(), "Used to read somas files")
        .def(py::init([](py::object h5_filepath) { return ReportType(py::str(h5_filepath)); }),
             "h5_filepath"_a)
        .def("get_population_names", &ReportType::getPopulationNames, "Get list of all populations")
        .def("__getitem__", &ReportType::openPopulation);
}


PYBIND11_MODULE(_libsonata, m) {
    py::class_<Selection>(m,
                          "Selection",
                          "ID sequence in the form convenient for querying attributes")
        .def(py::init<const Selection::Ranges&>(),
             py::arg("ranges"),
             "Selection from list of intervals")
        .def(py::init([](py::array_t<uint64_t, py::array::c_style> values) {
                 const auto raw = values.unchecked<1>();
                 return Selection::fromValues(raw.data(0), raw.data(raw.shape(0)));
             }),
             py::arg("values").noconvert(true),
             "Selection from list of IDs: passing np.array with dtype np.uint64 is faster")
        .def(py::init([](py::array_t<int64_t, py::array::c_style | py::array::forcecast> values) {
                 /* Both the Selection::Range and fromValues cases are handled here:
                  * It doesn't seem possible to prevent the cast of [(-1, 1), (2, 3) ... ] style
                  * Ranges; the Numpy auto-conversion from happening in this case.
                  */
                 py::buffer_info info = values.request();
                 if (info.ndim == 2) {
                     const auto raw = values.unchecked<2>();
                     Selection::Ranges ranges;
                     ranges.reserve(raw.shape(0));
                     for (py::ssize_t i = 0; i < raw.shape(0); ++i) {
                         if (raw(i, 0) < 0 || raw(i, 1) < 0) {
                             throw SonataError("Negative value passed to Selection");
                         }
                         ranges.emplace_back(raw(i, 0), raw(i, 1));
                     }
                     return Selection(ranges);
                 }
                 // one dimensional case; ie fromValues
                 const auto raw = values.unchecked<1>();
                 for (size_t i = 0; i < raw.shape(0); ++i) {
                     if (raw[i] < 0) {
                         throw SonataError("Negative value passed to Selection");
                     }
                 }

                 return Selection::fromValues(raw.data(0), raw.data(raw.shape(0)));
             }),
             "values"_a,
             "Selection from list of IDs: passing np.array with dtype np.uint64 is faster")
        .def_property_readonly("ranges", &Selection::ranges, DOC_SEL(ranges))
        .def(
            "flatten", [](Selection& obj) { return asArray(obj.flatten()); }, DOC_SEL(flatten))
        .def_property_readonly("flat_size", &Selection::flatSize, DOC_SEL(flatSize))
        .def(
            "__bool__",
            [](const Selection& obj) { return !obj.empty(); },
            "True if Selection is not empty")
        .def("__eq__", &bbp::sonata::operator==, "Compare selection contents are equal")
        .def("__ne__", &bbp::sonata::operator!=, "Compare selection contents are not equal")
        .def("__or__", &bbp::sonata::operator|, "Union of selections")
        .def("__and__", &bbp::sonata::operator&, "Intersection of selections")
        .def("__repr__", [](Selection& obj) {
            const auto ranges = obj.ranges();
            const size_t max_count = 10;

            if (ranges.size() < max_count) {
                return fmt::format("Selection([{}])", fmt::join(ranges, ", "));
            }

            return fmt::format("Selection([{}, ..., {}])",
                               fmt::join(ranges.begin(), ranges.begin() + 3, ", "),
                               fmt::join(ranges.end() - 3, ranges.end(), ", "));
        });
    py::implicitly_convertible<py::list, Selection>();
    py::implicitly_convertible<py::tuple, Selection>();

    bindPopulationClass<NodePopulation>(m, "NodePopulation", "Collection of nodes with attributes")
        .def(
            "match_values",
            [](NodePopulation& obj, const std::string& name, const size_t value) {
                return obj.matchAttributeValues(name, value);
            },
            "name"_a,
            "value"_a,
            DOC_POP_NODE(matchAttributeValues))
        .def(
            "match_values",
            [](NodePopulation& obj, const std::string& name, const std::string& value) {
                return obj.matchAttributeValues(name, value);
            },
            "name"_a,
            "value"_a,
            DOC_POP_NODE(matchAttributeValues));

    bindStorageClass<NodeStorage>(m, "NodeStorage", "NodePopulation");

    py::class_<NodeSets>(m, "NodeSets", "")
        .def(py::init<const std::string&>())
        .def_static("from_file", [](py::object path) { return NodeSets::fromFile(py::str(path)); })
        .def_property_readonly("names", &NodeSets::names, DOC_NODESETS(names))
        .def("materialize", &NodeSets::materialize, DOC_NODESETS(materialize))
        .def("toJSON", &NodeSets::toJSON, DOC_NODESETS(toJSON));

    py::class_<PopulationProperties>(m,
                                     "PopulationProperties",
                                     "Stores population-specific network information")
        .def_readonly("type", &PopulationProperties::type, "Population type")
        .def_readonly("biophysical_neuron_models_dir",
                      &PopulationProperties::biophysicalNeuronModelsDir,
                      "Path to the template HOC files defining the E-Mode")
        .def_readonly("morphologies_dir",
                      &PopulationProperties::morphologiesDir,
                      "Path to the directory containing the morphologies")
        .def_readonly("alternate_morphology_formats",
                      &PopulationProperties::alternateMorphologyFormats,
                      "Path to the directory containing the morphologies");

    py::class_<CircuitConfig>(m, "CircuitConfig", "")
        .def(py::init<const std::string&, const std::string&>())
        .def_static("from_file",
                    [](py::object path) { return CircuitConfig::fromFile(py::str(path)); })
        .def_property_readonly("node_sets_path", &CircuitConfig::getNodeSetsPath)
        .def_property_readonly("node_populations", &CircuitConfig::listNodePopulations)
        .def("node_population", &CircuitConfig::getNodePopulation)
        .def_property_readonly("edge_populations", &CircuitConfig::listEdgePopulations)
        .def("edge_population", &CircuitConfig::getEdgePopulation)
        .def("node_population_properties", &CircuitConfig::getNodePopulationProperties, "name"_a)
        .def("edge_population_properties", &CircuitConfig::getEdgePopulationProperties, "name"_a)
        .def_property_readonly("expanded_json", &CircuitConfig::getExpandedJSON);

    py::class_<SimulationConfig::Run>(m,
                                      "Run",
                                      "Stores parameters defining global simulation settings")
        .def_readonly("tstop",
                      &SimulationConfig::Run::tstop,
                      "Biological simulation end time in milliseconds")
        .def_readonly("dt",
                      &SimulationConfig::Run::dt,
                      "Simulation integration step in milliseconds");

    py::class_<SimulationConfig::Output>(m,
                                         "Output",
                                         "Stores overriden parameters of simulation output")
        .def_readonly("output_dir",
                      &SimulationConfig::Output::outputDir,
                      "Simulation output directory")
        .def_readonly("spikes_file",
                      &SimulationConfig::Output::spikesFile,
                      "Spike report filename");

    py::class_<SimulationConfig::Report>(m, "Report", "List of parameters of a report")
        .def_readonly("cells", &SimulationConfig::Report::cells, "Node sets on which to report")
        .def_readonly("type",
                      &SimulationConfig::Report::type,
                      "Report type. Possible values are 'compartment', 'summation', 'synapse")
        .def_readonly("dt",
                      &SimulationConfig::Report::dt,
                      "Interval between reporting steps in milliseconds")
        .def_readonly("start_time",
                      &SimulationConfig::Report::startTime,
                      "Time to start reporting, in milliseconds")
        .def_readonly("end_time",
                      &SimulationConfig::Report::endTime,
                      "Time to stop reporting in milliseconds")
        .def_readonly("file_name", &SimulationConfig::Report::fileName, "Report file name");

    py::class_<SimulationConfig>(m, "SimulationConfig", "")
        .def(py::init<const std::string&, const std::string&>())
        .def_static("from_file",
                    [](py::object path) { return SimulationConfig::fromFile(py::str(path)); })
        .def_property_readonly("base_path", &SimulationConfig::getBasePath)
        .def_property_readonly("json", &SimulationConfig::getJSON)
        .def_property_readonly("run", &SimulationConfig::getRun)
        .def_property_readonly("output", &SimulationConfig::getOutput)
        .def("report", &SimulationConfig::getReport, "name"_a);

    bindPopulationClass<EdgePopulation>(
        m, "EdgePopulation", "Collection of edges with attributes and connectivity index")
        .def_property_readonly("source", &EdgePopulation::source, DOC_POP_EDGE(source))
        .def_property_readonly("target", &EdgePopulation::target, DOC_POP_EDGE(target))
        .def(
            "source_node",
            [](EdgePopulation& obj, EdgeID edgeID) {
                return obj.sourceNodeIDs(Selection::fromValues({edgeID}))[0];
            },
            "edge_id"_a,
            "Source node ID for a given edge")
        .def(
            "source_nodes",
            [](EdgePopulation& obj, const Selection& selection) {
                return asArray(obj.sourceNodeIDs(selection));
            },
            "selection"_a,
            DOC_POP_EDGE(sourceNodeIDs))
        .def(
            "target_node",
            [](EdgePopulation& obj, EdgeID edgeID) {
                return obj.targetNodeIDs(Selection::fromValues({edgeID}))[0];
            },
            "edge_id"_a,
            "Target node ID for given edge")
        .def(
            "target_nodes",
            [](EdgePopulation& obj, const Selection& selection) {
                return asArray(obj.targetNodeIDs(selection));
            },
            "selection"_a,
            DOC_POP_EDGE(targetNodeIDs))
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, NodeID target) {
                return obj.afferentEdges(std::vector<NodeID>{target});
            },
            "target"_a,
            DOC_POP_EDGE(afferentEdges))
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& target) {
                return obj.afferentEdges(target);
            },
            "target"_a,
            DOC_POP_EDGE(afferentEdges))
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, NodeID source) {
                return obj.efferentEdges(std::vector<NodeID>{source});
            },
            "source"_a,
            DOC_POP_EDGE(efferentEdges))
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& source) {
                return obj.efferentEdges(source);
            },
            "source"_a,
            DOC_POP_EDGE(efferentEdges))
        .def(
            "connecting_edges",
            [](EdgePopulation& obj, NodeID source, NodeID target) {
                return obj.connectingEdges(std::vector<NodeID>{source},
                                           std::vector<NodeID>{target});
            },
            "source"_a,
            "target"_a,
            DOC_POP_EDGE(connectingEdges))
        .def(
            "connecting_edges",
            [](EdgePopulation& obj,
               const std::vector<NodeID>& source,
               const std::vector<NodeID>& target) { return obj.connectingEdges(source, target); },
            "source"_a,
            "target"_a,
            "Find all edges connecting two given node sets")
        .def_static("write_indices",
                    &EdgePopulation::writeIndices,
                    "h5_filepath"_a,
                    "population"_a,
                    "source_node_count"_a,
                    "target_node_count"_a,
                    "overwrite"_a = false,
                    DOC_POP_EDGE(writeIndices));

    bindStorageClass<EdgeStorage>(m, "EdgeStorage", "EdgePopulation");

    py::class_<SpikeReader::Population>(m, "SpikePopulation", "A population inside a SpikeReader")
        .def("get",
             &SpikeReader::Population::get,
             "Return spikes with all those node_ids between 'tstart' and 'tstop'",
             "node_ids"_a = nonstd::nullopt,
             "tstart"_a = nonstd::nullopt,
             "tstop"_a = nonstd::nullopt)
        .def_property_readonly(
            "sorting",
            [](const SpikeReader::Population& self) {
                auto s = self.getSorting();
                if (s == SpikeReader::Population::Sorting::by_id)
                    return "by_id";
                if (s == SpikeReader::Population::Sorting::by_time)
                    return "by_time";
                return "none";
            },
            DOC_SPIKEREADER_POP(getSorting))
        .def_property_readonly("times",
                               &SpikeReader::Population::getTimes,
                               DOC_SPIKEREADER_POP(getTimes));
    py::class_<SpikeReader>(m, "SpikeReader", "Used to read spike files")
        .def(py::init([](py::object h5_filepath) { return SpikeReader(py::str(h5_filepath)); }),
             "h5_filepath"_a)
        .def("get_population_names",
             &SpikeReader::getPopulationNames,
             DOC_SPIKEREADER(getPopulationNames))
        .def("__getitem__", &SpikeReader::openPopulation);

    bindReportReader<SomaReportReader, NodeID>(m, "Soma");
    bindReportReader<ElementReportReader, CompartmentID>(m, "Element");

    py::register_exception<SonataError>(m, "SonataError");
}
