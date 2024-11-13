#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <bbp/sonata/common.h>
#include <bbp/sonata/config.h>
#include <bbp/sonata/edges.h>
#include <bbp/sonata/node_sets.h>
#include <bbp/sonata/nodes.h>
#include <bbp/sonata/optional.hpp>  //nonstd::optional
#include <bbp/sonata/report_reader.h>
#include <bbp/sonata/variant.hpp>  //nonstd::variant

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
#define DOC_COMMON_POPULATION_PROPERTIES(x) DOC(bbp, sonata, CommonPopulationProperties, x)
#define DOC_NODE_POPULATION_PROPERTIES(x) DOC(bbp, sonata, NodePopulationProperties, x)
#define DOC_EDGE_POPULATION_PROPERTIES(x) DOC(bbp, sonata, EdgePopulationProperties, x)
#define DOC_SIMULATIONCONFIG(...) DOC(bbp, sonata, SimulationConfig, __VA_ARGS__)

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
        .def(py::init([](py::object h5_filepath, py::object csv_filepath, Hdf5Reader hdf5_reader) {
                 return Storage(py::str(h5_filepath),
                                py::str(csv_filepath),
                                std::move(hdf5_reader));
             }),
             "h5_filepath"_a,
             "csv_filepath"_a = "",
             "hdf5_reader"_a = Hdf5Reader())
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

template <typename... Ts>
struct type_caster<nonstd::variant<Ts...>>: variant_caster<nonstd::variant<Ts...>> {};
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
             "tstride"_a = nonstd::nullopt,
             "block_gap_limit"_a = nonstd::nullopt)
        .def("get_node_ids",
             &ReportType::Population::getNodeIds,
             "Return the list of nodes ids for this population")
        .def(
            "get_node_id_element_id_mapping",
            [](const typename ReportType::Population& population,
               const nonstd::optional<Selection>& selection,
               const nonstd::optional<size_t>& block_gap_limit) {
                return asArray(population.getNodeIdElementIdMapping(selection, block_gap_limit));
            },
            DOC_REPORTREADER_POP(getNodeIdElementIdMapping),
            "selection"_a = nonstd::nullopt,
            "block_gap_limit"_a = nonstd::nullopt)
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
    py::class_<Hdf5Reader>(m, "Hdf5Reader").def(py::init([]() { return Hdf5Reader(); }));

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
                         ranges.push_back({uint64_t(raw(i, 0)), uint64_t(raw(i, 1))});
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
        .def_property_readonly(
            "ranges",
            [](const Selection& obj) {
                const auto& ranges = obj.ranges();
                auto list = py::list(ranges.size());
                for (size_t i = 0; i < ranges.size(); ++i) {
                    const auto& range = ranges[i];
                    list[i] = py::make_tuple(std::get<0>(range), std::get<1>(range));
                }

                return list;
            },
            DOC_SEL(ranges))
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
            const auto& ranges = obj.ranges();
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
            [](NodePopulation& obj, const std::string& name, const std::vector<size_t>& values) {
                return obj.matchAttributeValues(name, values);
            },
            "name"_a,
            "value"_a,
            DOC_POP_NODE(matchAttributeValues))
        .def(
            "match_values",
            [](NodePopulation& obj,
               const std::string& name,
               const std::vector<std::string>& values) {
                return obj.matchAttributeValues(name, values);
            },
            "name"_a,
            "value"_a,
            DOC_POP_NODE(matchAttributeValues))
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

    py::class_<NodeSets>(m, "NodeSets", "NodeSets")
        .def(py::init<const std::string&>())
        .def_static("from_file", [](py::object path) { return NodeSets::fromFile(py::str(path)); })
        .def_property_readonly("names", &NodeSets::names, DOC_NODESETS(names))
        .def("materialize", &NodeSets::materialize, DOC_NODESETS(materialize))
        .def("update", &NodeSets::update, "other"_a, DOC_NODESETS(update))
        .def("toJSON", &NodeSets::toJSON, DOC_NODESETS(toJSON));

    py::class_<CommonPopulationProperties>(m,
                                           "CommonPopulationProperties",
                                           "Stores population-specific network information")
        .def_readonly("type",
                      &CommonPopulationProperties::type,
                      DOC_COMMON_POPULATION_PROPERTIES(type))
        .def_readonly("biophysical_neuron_models_dir",
                      &CommonPopulationProperties::biophysicalNeuronModelsDir,
                      DOC_COMMON_POPULATION_PROPERTIES(biophysicalNeuronModelsDir))
        .def_readonly("morphologies_dir",
                      &CommonPopulationProperties::morphologiesDir,
                      DOC_COMMON_POPULATION_PROPERTIES(morphologiesDir))
        .def_readonly("alternate_morphology_formats",
                      &CommonPopulationProperties::alternateMorphologyFormats,
                      DOC_COMMON_POPULATION_PROPERTIES(alternateMorphologyFormats))
        .def_readonly("elements_path",
                      &CommonPopulationProperties::elementsPath,
                      DOC_COMMON_POPULATION_PROPERTIES(elementsPath))
        .def_readonly("types_path",
                      &CommonPopulationProperties::typesPath,
                      DOC_COMMON_POPULATION_PROPERTIES(typesPath));

    py::class_<NodePopulationProperties, CommonPopulationProperties>(
        m, "NodePopulationProperties", "Stores node population-specific network information")
        .def_readonly("spatial_segment_index_dir",
                      &NodePopulationProperties::spatialSegmentIndexDir,
                      DOC_NODE_POPULATION_PROPERTIES(spatialSegmentIndexDir))
        .def_readonly("vasculature_file",
                      &NodePopulationProperties::vasculatureFile,
                      DOC_NODE_POPULATION_PROPERTIES(vasculatureFile))
        .def_readonly("vasculature_mesh",
                      &NodePopulationProperties::vasculatureMesh,
                      DOC_NODE_POPULATION_PROPERTIES(vasculatureMesh))
        .def_readonly("microdomains_file",
                      &NodePopulationProperties::microdomainsFile,
                      DOC_NODE_POPULATION_PROPERTIES(microdomainsFile));

    py::class_<EdgePopulationProperties, CommonPopulationProperties>(
        m, "EdgePopulationProperties", "Stores edge population-specific network information")
        .def_readonly("spatial_synapse_index_dir",
                      &EdgePopulationProperties::spatialSynapseIndexDir,
                      DOC_EDGE_POPULATION_PROPERTIES(spatialSynapseIndexDir))
        .def_readonly("endfeet_meshes_file",
                      &EdgePopulationProperties::endfeetMeshesFile,
                      DOC_EDGE_POPULATION_PROPERTIES(endfeetMeshesFile))
        .def_readonly("spine_morphologies_dir",
                      &EdgePopulationProperties::spineMorphologiesDir,
                      DOC_EDGE_POPULATION_PROPERTIES(spineMorphologiesDir));

    py::enum_<CircuitConfig::ConfigStatus>(m, "CircuitConfigStatus")
        .value("invalid", CircuitConfig::ConfigStatus::invalid)
        .value("complete", CircuitConfig::ConfigStatus::complete)
        .value("partial", CircuitConfig::ConfigStatus::partial);

    py::class_<CircuitConfig>(m, "CircuitConfig", "Circuit Configuration")
        .def(py::init<const std::string&, const std::string&>())
        .def_static("from_file",
                    [](py::object path) { return CircuitConfig::fromFile(py::str(path)); })
        .def_property_readonly("config_status", &CircuitConfig::getCircuitConfigStatus, "ibid")
        .def_property_readonly("node_sets_path", &CircuitConfig::getNodeSetsPath)
        .def_property_readonly("node_populations", &CircuitConfig::listNodePopulations)
        .def("node_population",
             [](const CircuitConfig& config, const std::string& name) {
                 return config.getNodePopulation(name);
             })
        .def_property_readonly("edge_populations", &CircuitConfig::listEdgePopulations)
        .def("edge_population",
             [](const CircuitConfig& config, const std::string& name) {
                 return config.getEdgePopulation(name);
             })
        .def("edge_population",
             [](const CircuitConfig& config, const std::string& name, Hdf5Reader hdf5_reader) {
                 return config.getEdgePopulation(name, hdf5_reader);
             })
        .def("node_population_properties", &CircuitConfig::getNodePopulationProperties, "name"_a)
        .def("edge_population_properties", &CircuitConfig::getEdgePopulationProperties, "name"_a)
        .def_property_readonly("expanded_json", &CircuitConfig::getExpandedJSON);

    py::class_<SimulationConfig> simConf(m, "SimulationConfig", "Simulation Configuration");
    py::class_<SimulationConfig::Run> run(simConf,
                                          "Run",
                                          "Stores parameters defining global simulation settings");
    run.def_readonly("tstop", &SimulationConfig::Run::tstop, DOC_SIMULATIONCONFIG(Run, tstop))
        .def_readonly("dt", &SimulationConfig::Run::dt, DOC_SIMULATIONCONFIG(Run, dt))
        .def_readonly("random_seed",
                      &SimulationConfig::Run::randomSeed,
                      DOC_SIMULATIONCONFIG(Run, randomSeed))
        .def_readonly("spike_threshold",
                      &SimulationConfig::Run::spikeThreshold,
                      DOC_SIMULATIONCONFIG(Run, spikeThreshold))
        .def_readonly("integration_method",
                      &SimulationConfig::Run::integrationMethod,
                      DOC_SIMULATIONCONFIG(Run, integrationMethod))
        .def_readonly("stimulus_seed",
                      &SimulationConfig::Run::stimulusSeed,
                      DOC_SIMULATIONCONFIG(Run, stimulusSeed))
        .def_readonly("ionchannel_seed",
                      &SimulationConfig::Run::ionchannelSeed,
                      DOC_SIMULATIONCONFIG(Run, ionchannelSeed))
        .def_readonly("minis_seed",
                      &SimulationConfig::Run::minisSeed,
                      DOC_SIMULATIONCONFIG(Run, minisSeed))
        .def_readonly("synapse_seed",
                      &SimulationConfig::Run::synapseSeed,
                      DOC_SIMULATIONCONFIG(Run, synapseSeed))
        .def_readonly("electrodes_file",
                      &SimulationConfig::Run::electrodesFile,
                      DOC_SIMULATIONCONFIG(Run, electrodesFile));

    py::enum_<SimulationConfig::Run::IntegrationMethod>(run, "IntegrationMethod")
        .value("euler", SimulationConfig::Run::IntegrationMethod::euler)
        .value("crank_nicolson", SimulationConfig::Run::IntegrationMethod::crank_nicolson)
        .value("crank_nicolson_ion", SimulationConfig::Run::IntegrationMethod::crank_nicolson_ion);

    py::class_<SimulationConfig::Output> output(simConf,
                                                "Output",
                                                "Parameters of simulation output");
    output
        .def_readonly("output_dir",
                      &SimulationConfig::Output::outputDir,
                      DOC_SIMULATIONCONFIG(Output, outputDir))
        .def_readonly("spikes_file",
                      &SimulationConfig::Output::spikesFile,
                      DOC_SIMULATIONCONFIG(Output, spikesFile))
        .def_readonly("log_file",
                      &SimulationConfig::Output::logFile,
                      DOC_SIMULATIONCONFIG(Output, logFile))
        .def_readonly("spikes_file",
                      &SimulationConfig::Output::spikesFile,
                      DOC_SIMULATIONCONFIG(Output, spikesFile))
        .def_readonly("spikes_sort_order",
                      &SimulationConfig::Output::sortOrder,
                      DOC_SIMULATIONCONFIG(Output, sortOrder));

    py::enum_<SimulationConfig::Output::SpikesSortOrder>(output, "SpikesSortOrder")
        .value("none", SimulationConfig::Output::SpikesSortOrder::none)
        .value("by_id", SimulationConfig::Output::SpikesSortOrder::by_id)
        .value("by_time", SimulationConfig::Output::SpikesSortOrder::by_time);

    py::class_<SimulationConfig::Conditions> conditions(
        simConf, "Conditions", "Parameters defining global experimental conditions");
    conditions
        .def_readonly("celsius",
                      &SimulationConfig::Conditions::celsius,
                      DOC_SIMULATIONCONFIG(Conditions, celsius))
        .def_readonly("v_init",
                      &SimulationConfig::Conditions::vInit,
                      DOC_SIMULATIONCONFIG(Conditions, vInit))
        .def_readonly("spike_location",
                      &SimulationConfig::Conditions::spikeLocation,
                      DOC_SIMULATIONCONFIG(Conditions, spikeLocation))
        .def_readonly("extracellular_calcium",
                      &SimulationConfig::Conditions::extracellularCalcium,
                      DOC_SIMULATIONCONFIG(Conditions, extracellularCalcium))
        .def_readonly("randomize_gaba_rise_time",
                      &SimulationConfig::Conditions::randomizeGabaRiseTime,
                      DOC_SIMULATIONCONFIG(Conditions, randomizeGabaRiseTime))
        .def_readonly("mechanisms",
                      &SimulationConfig::Conditions::mechanisms,
                      DOC_SIMULATIONCONFIG(Conditions, mechanisms))
        .def("modifications",
             &SimulationConfig::Conditions::getModifications,
             DOC_SIMULATIONCONFIG(Conditions, getModifications));


    py::enum_<SimulationConfig::Conditions::SpikeLocation>(conditions, "SpikeLocation")
        .value("soma", SimulationConfig::Conditions::SpikeLocation::soma)
        .value("AIS", SimulationConfig::Conditions::SpikeLocation::AIS);

    py::class_<SimulationConfig::ModificationBase> modificationBase(simConf, "ModificationBase");
    modificationBase
        .def_readonly("name",
                      &SimulationConfig::ModificationBase::name,
                      DOC_SIMULATIONCONFIG(ModificationBase, name))
        .def_readonly("node_set",
                      &SimulationConfig::ModificationBase::nodeSet,
                      DOC_SIMULATIONCONFIG(ModificationBase, nodeSet))
        .def_readonly("type",
                      &SimulationConfig::ModificationBase::type,
                      DOC_SIMULATIONCONFIG(ModificationBase, type));

    py::class_<SimulationConfig::ModificationTTX, SimulationConfig::ModificationBase>(
        simConf, "ModificationTTX");

    py::class_<SimulationConfig::ModificationConfigureAllSections,
               SimulationConfig::ModificationBase>(simConf, "ModificationConfigureAllSections")
        .def_readonly("section_configure",
                      &SimulationConfig::ModificationConfigureAllSections::sectionConfigure,
                      DOC_SIMULATIONCONFIG(ModificationConfigureAllSections, sectionConfigure));

    py::enum_<SimulationConfig::ModificationBase::ModificationType>(modificationBase,
                                                                    "ModificationType")
        .value("TTX",
               SimulationConfig::ModificationBase::ModificationType::TTX,
               DOC_SIMULATIONCONFIG(ModificationBase, ModificationType, TTX))
        .value("ConfigureAllSections",
               SimulationConfig::ModificationBase::ModificationType::ConfigureAllSections,
               DOC_SIMULATIONCONFIG(ModificationBase, ModificationType, ConfigureAllSections));

    py::class_<SimulationConfig::Report> report(simConf, "Report", "Parameters of a report");
    report
        .def_readonly("cells",
                      &SimulationConfig::Report::cells,
                      DOC_SIMULATIONCONFIG(Report, cells))
        .def_readonly("sections",
                      &SimulationConfig::Report::sections,
                      DOC_SIMULATIONCONFIG(Report, sections))
        .def_readonly("type", &SimulationConfig::Report::type, DOC_SIMULATIONCONFIG(Report, type))
        .def_readonly("scaling",
                      &SimulationConfig::Report::scaling,
                      DOC_SIMULATIONCONFIG(Report, scaling))
        .def_readonly("compartments",
                      &SimulationConfig::Report::compartments,
                      DOC_SIMULATIONCONFIG(Report, compartments))
        .def_readonly("variable_name",
                      &SimulationConfig::Report::variableName,
                      DOC_SIMULATIONCONFIG(Report, variableName))
        .def_readonly("unit", &SimulationConfig::Report::unit, DOC_SIMULATIONCONFIG(Report, unit))
        .def_readonly("dt", &SimulationConfig::Report::dt, DOC_SIMULATIONCONFIG(Report, dt))
        .def_readonly("start_time",
                      &SimulationConfig::Report::startTime,
                      DOC_SIMULATIONCONFIG(Report, startTime))
        .def_readonly("end_time",
                      &SimulationConfig::Report::endTime,
                      DOC_SIMULATIONCONFIG(Report, endTime))
        .def_readonly("file_name",
                      &SimulationConfig::Report::fileName,
                      DOC_SIMULATIONCONFIG(Report, fileName))
        .def_readonly("enabled",
                      &SimulationConfig::Report::enabled,
                      DOC_SIMULATIONCONFIG(Report, enabled));

    py::enum_<SimulationConfig::Report::Sections>(report, "Sections")
        .value("soma",
               SimulationConfig::Report::Sections::soma,
               DOC_SIMULATIONCONFIG(Report, Sections, soma))
        .value("axon",
               SimulationConfig::Report::Sections::axon,
               DOC_SIMULATIONCONFIG(Report, Sections, soma))
        .value("dend",
               SimulationConfig::Report::Sections::dend,
               DOC_SIMULATIONCONFIG(Report, Sections, soma))
        .value("apic",
               SimulationConfig::Report::Sections::apic,
               DOC_SIMULATIONCONFIG(Report, Sections, soma))
        .value("all",
               SimulationConfig::Report::Sections::all,
               DOC_SIMULATIONCONFIG(Report, Sections, soma));

    py::enum_<SimulationConfig::Report::Type>(report, "Type")
        .value("compartment", SimulationConfig::Report::Type::compartment)
        .value("lfp", SimulationConfig::Report::Type::lfp)
        .value("summation", SimulationConfig::Report::Type::summation)
        .value("synapse", SimulationConfig::Report::Type::synapse);

    py::enum_<SimulationConfig::Report::Scaling>(report, "Scaling")
        .value("none", SimulationConfig::Report::Scaling::none)
        .value("area", SimulationConfig::Report::Scaling::area);

    py::enum_<SimulationConfig::Report::Compartments>(report, "Compartments")
        .value("center", SimulationConfig::Report::Compartments::center)
        .value("all", SimulationConfig::Report::Compartments::all);

    py::class_<SimulationConfig::InputBase> inputBase(simConf, "InputBase");
    inputBase
        .def_readonly("module",
                      &SimulationConfig::InputBase::module,
                      DOC_SIMULATIONCONFIG(InputBase, module))
        .def_readonly("input_type",
                      &SimulationConfig::InputBase::inputType,
                      DOC_SIMULATIONCONFIG(InputBase, inputType))
        .def_readonly("delay",
                      &SimulationConfig::InputBase::delay,
                      DOC_SIMULATIONCONFIG(InputBase, delay))
        .def_readonly("duration",
                      &SimulationConfig::InputBase::duration,
                      DOC_SIMULATIONCONFIG(InputBase, duration))
        .def_readonly("node_set",
                      &SimulationConfig::InputBase::nodeSet,
                      DOC_SIMULATIONCONFIG(InputBase, nodeSet));

    py::class_<SimulationConfig::InputLinear, SimulationConfig::InputBase>(simConf, "Linear")
        .def_readonly("amp_start",
                      &SimulationConfig::InputLinear::ampStart,
                      DOC_SIMULATIONCONFIG(InputLinear, ampStart))
        .def_readonly("amp_end",
                      &SimulationConfig::InputLinear::ampEnd,
                      DOC_SIMULATIONCONFIG(InputLinear, ampEnd))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputLinear::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputLinear, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputRelativeLinear, SimulationConfig::InputBase>(simConf,
                                                                                   "RelativeLinear")
        .def_readonly("percent_start",
                      &SimulationConfig::InputRelativeLinear::percentStart,
                      DOC_SIMULATIONCONFIG(InputRelativeLinear, percentStart))
        .def_readonly("percent_end",
                      &SimulationConfig::InputRelativeLinear::percentEnd,
                      DOC_SIMULATIONCONFIG(InputRelativeLinear, percentEnd))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputRelativeLinear::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputRelativeLinear, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputPulse, SimulationConfig::InputBase>(simConf, "Pulse")
        .def_readonly("amp_start",
                      &SimulationConfig::InputPulse::ampStart,
                      DOC_SIMULATIONCONFIG(InputPulse, ampStart))
        .def_readonly("width",
                      &SimulationConfig::InputPulse::width,
                      DOC_SIMULATIONCONFIG(InputPulse, width))
        .def_readonly("frequency",
                      &SimulationConfig::InputPulse::frequency,
                      DOC_SIMULATIONCONFIG(InputPulse, frequency))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputPulse::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputPulse, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputSinusoidal, SimulationConfig::InputBase>(simConf,
                                                                               "Sinusoidal")
        .def_readonly("amp_start",
                      &SimulationConfig::InputSinusoidal::ampStart,
                      DOC_SIMULATIONCONFIG(InputSinusoidal, ampStart))
        .def_readonly("frequency",
                      &SimulationConfig::InputSinusoidal::frequency,
                      DOC_SIMULATIONCONFIG(InputSinusoidal, frequency))
        .def_readonly("dt",
                      &SimulationConfig::InputSinusoidal::dt,
                      DOC_SIMULATIONCONFIG(InputSinusoidal, dt))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputSinusoidal::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputSinusoidal, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputSubthreshold, SimulationConfig::InputBase>(simConf,
                                                                                 "Subthreshold")
        .def_readonly("percent_less",
                      &SimulationConfig::InputSubthreshold::percentLess,
                      DOC_SIMULATIONCONFIG(InputSubthreshold, percentLess))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputSubthreshold::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputSubthreshold, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputHyperpolarizing, SimulationConfig::InputBase>(
        simConf, "Hyperpolarizing")
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputHyperpolarizing::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputHyperpolarizing, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputSynapseReplay, SimulationConfig::InputBase>(simConf,
                                                                                  "SynapseReplay")
        .def_readonly("spike_file",
                      &SimulationConfig::InputSynapseReplay::spikeFile,
                      DOC_SIMULATIONCONFIG(InputSynapseReplay, spikeFile));

    py::class_<SimulationConfig::InputSeclamp, SimulationConfig::InputBase>(simConf, "Seclamp")
        .def_readonly("voltage",
                      &SimulationConfig::InputSeclamp::voltage,
                      DOC_SIMULATIONCONFIG(InputSeclamp, voltage))
        .def_readonly("series_resistance",
                      &SimulationConfig::InputSeclamp::seriesResistance,
                      DOC_SIMULATIONCONFIG(InputSeclamp, seriesResistance));

    py::class_<SimulationConfig::InputNoise, SimulationConfig::InputBase>(simConf, "Noise")
        .def_readonly("mean",
                      &SimulationConfig::InputNoise::mean,
                      DOC_SIMULATIONCONFIG(InputNoise, mean))
        .def_readonly("mean_percent",
                      &SimulationConfig::InputNoise::meanPercent,
                      DOC_SIMULATIONCONFIG(InputNoise, meanPercent))
        .def_readonly("variance",
                      &SimulationConfig::InputNoise::variance,
                      DOC_SIMULATIONCONFIG(InputNoise, variance))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputNoise::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputNoise, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputShotNoise, SimulationConfig::InputBase>(simConf, "ShotNoise")
        .def_readonly("rise_time",
                      &SimulationConfig::InputShotNoise::riseTime,
                      DOC_SIMULATIONCONFIG(InputShotNoise, riseTime))
        .def_readonly("decay_time",
                      &SimulationConfig::InputShotNoise::decayTime,
                      DOC_SIMULATIONCONFIG(InputShotNoise, decayTime))
        .def_readonly("random_seed",
                      &SimulationConfig::InputShotNoise::randomSeed,
                      DOC_SIMULATIONCONFIG(InputShotNoise, randomSeed))
        .def_readonly("reversal",
                      &SimulationConfig::InputShotNoise::reversal,
                      DOC_SIMULATIONCONFIG(InputShotNoise, reversal))
        .def_readonly("dt",
                      &SimulationConfig::InputShotNoise::dt,
                      DOC_SIMULATIONCONFIG(InputShotNoise, dt))
        .def_readonly("rate",
                      &SimulationConfig::InputShotNoise::rate,
                      DOC_SIMULATIONCONFIG(InputShotNoise, rate))
        .def_readonly("amp_mean",
                      &SimulationConfig::InputShotNoise::ampMean,
                      DOC_SIMULATIONCONFIG(InputShotNoise, ampMean))
        .def_readonly("amp_var",
                      &SimulationConfig::InputShotNoise::ampVar,
                      DOC_SIMULATIONCONFIG(InputShotNoise, ampVar))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputShotNoise::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputShotNoise, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputRelativeShotNoise, SimulationConfig::InputBase>(
        simConf, "RelativeShotNoise")
        .def_readonly("rise_time",
                      &SimulationConfig::InputRelativeShotNoise::riseTime,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, riseTime))
        .def_readonly("decay_time",
                      &SimulationConfig::InputRelativeShotNoise::decayTime,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, decayTime))
        .def_readonly("random_seed",
                      &SimulationConfig::InputRelativeShotNoise::randomSeed,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, randomSeed))
        .def_readonly("reversal",
                      &SimulationConfig::InputRelativeShotNoise::reversal,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, reversal))
        .def_readonly("dt",
                      &SimulationConfig::InputRelativeShotNoise::dt,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, dt))
        .def_readonly("sd_percent",
                      &SimulationConfig::InputRelativeShotNoise::sdPercent,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, sdPercent))
        .def_readonly("mean_percent",
                      &SimulationConfig::InputRelativeShotNoise::meanPercent,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, meanPercent))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputRelativeShotNoise::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, representsPhysicalElectrode))
        .def_readonly("relative_skew",
                      &SimulationConfig::InputRelativeShotNoise::relativeSkew,
                      DOC_SIMULATIONCONFIG(InputRelativeShotNoise, relativeSkew));

    py::class_<SimulationConfig::InputAbsoluteShotNoise, SimulationConfig::InputBase>(
        simConf, "AbsoluteShotNoise")
        .def_readonly("rise_time",
                      &SimulationConfig::InputAbsoluteShotNoise::riseTime,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, riseTime))
        .def_readonly("decay_time",
                      &SimulationConfig::InputAbsoluteShotNoise::decayTime,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, decayTime))
        .def_readonly("random_seed",
                      &SimulationConfig::InputAbsoluteShotNoise::randomSeed,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, randomSeed))
        .def_readonly("reversal",
                      &SimulationConfig::InputAbsoluteShotNoise::reversal,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, reversal))
        .def_readonly("dt",
                      &SimulationConfig::InputAbsoluteShotNoise::dt,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, dt))
        .def_readonly("mean",
                      &SimulationConfig::InputAbsoluteShotNoise::mean,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, mean))
        .def_readonly("sigma",
                      &SimulationConfig::InputAbsoluteShotNoise::sigma,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, sigma))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputAbsoluteShotNoise::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, representsPhysicalElectrode))
        .def_readonly("relative_skew",
                      &SimulationConfig::InputAbsoluteShotNoise::relativeSkew,
                      DOC_SIMULATIONCONFIG(InputAbsoluteShotNoise, relativeSkew));
    ;

    py::class_<SimulationConfig::InputOrnsteinUhlenbeck, SimulationConfig::InputBase>(
        simConf, "OrnsteinUhlenbeck")
        .def_readonly("tau",
                      &SimulationConfig::InputOrnsteinUhlenbeck::tau,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, tau))
        .def_readonly("reversal",
                      &SimulationConfig::InputOrnsteinUhlenbeck::reversal,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, reversal))
        .def_readonly("dt",
                      &SimulationConfig::InputOrnsteinUhlenbeck::dt,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, dt))
        .def_readonly("random_seed",
                      &SimulationConfig::InputOrnsteinUhlenbeck::randomSeed,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, randomSeed))
        .def_readonly("mean",
                      &SimulationConfig::InputOrnsteinUhlenbeck::mean,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, mean))
        .def_readonly("sigma",
                      &SimulationConfig::InputOrnsteinUhlenbeck::sigma,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, sigma))
        .def_readonly("represents_physical_electrode",
                      &SimulationConfig::InputOrnsteinUhlenbeck::representsPhysicalElectrode,
                      DOC_SIMULATIONCONFIG(InputOrnsteinUhlenbeck, representsPhysicalElectrode));

    py::class_<SimulationConfig::InputRelativeOrnsteinUhlenbeck, SimulationConfig::InputBase>(
        simConf, "RelativeOrnsteinUhlenbeck")
        .def_readonly("tau",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::tau,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, tau))
        .def_readonly("reversal",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::reversal,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, reversal))
        .def_readonly("dt",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::dt,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, dt))
        .def_readonly("random_seed",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::randomSeed,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, randomSeed))
        .def_readonly("mean_percent",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::meanPercent,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, meanPercent))
        .def_readonly("sd_percent",
                      &SimulationConfig::InputRelativeOrnsteinUhlenbeck::sdPercent,
                      DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, sdPercent))
        .def_readonly(
            "represents_physical_electrode",
            &SimulationConfig::InputRelativeOrnsteinUhlenbeck::representsPhysicalElectrode,
            DOC_SIMULATIONCONFIG(InputRelativeOrnsteinUhlenbeck, representsPhysicalElectrode));

    py::enum_<SimulationConfig::InputBase::Module>(inputBase, "Module")
        .value("linear", SimulationConfig::InputBase::Module::linear)
        .value("relative_linear", SimulationConfig::InputBase::Module::relative_linear)
        .value("pulse", SimulationConfig::InputBase::Module::pulse)
        .value("sinusoidal", SimulationConfig::InputBase::Module::sinusoidal)
        .value("subthreshold", SimulationConfig::InputBase::Module::subthreshold)
        .value("hyperpolarizing", SimulationConfig::InputBase::Module::hyperpolarizing)
        .value("synapse_replay", SimulationConfig::InputBase::Module::synapse_replay)
        .value("seclamp", SimulationConfig::InputBase::Module::seclamp)
        .value("noise", SimulationConfig::InputBase::Module::noise)
        .value("shot_noise", SimulationConfig::InputBase::Module::shot_noise)
        .value("relative_shot_noise", SimulationConfig::InputBase::Module::relative_shot_noise)
        .value("absolute_shot_noise", SimulationConfig::InputBase::Module::absolute_shot_noise)
        .value("ornstein_uhlenbeck", SimulationConfig::InputBase::Module::ornstein_uhlenbeck)
        .value("relative_ornstein_uhlenbeck",
               SimulationConfig::InputBase::Module::relative_ornstein_uhlenbeck);

    py::enum_<SimulationConfig::InputBase::InputType>(inputBase, "InputType")
        .value("spikes", SimulationConfig::InputBase::InputType::spikes)
        .value("extracellular_stimulation",
               SimulationConfig::InputBase::InputType::extracellular_stimulation)
        .value("current_clamp", SimulationConfig::InputBase::InputType::current_clamp)
        .value("voltage_clamp", SimulationConfig::InputBase::InputType::voltage_clamp)
        .value("conductance", SimulationConfig::InputBase::InputType::conductance);

    py::class_<SimulationConfig::ConnectionOverride>(simConf,
                                                     "ConnectionOverride",
                                                     "List of parameters of a connection")
        .def_readonly("name",
                      &SimulationConfig::ConnectionOverride::name,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, name))
        .def_readonly("source",
                      &SimulationConfig::ConnectionOverride::source,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, source))
        .def_readonly("target",
                      &SimulationConfig::ConnectionOverride::target,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, target))
        .def_readonly("weight",
                      &SimulationConfig::ConnectionOverride::weight,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, weight))
        .def_readonly("spont_minis",
                      &SimulationConfig::ConnectionOverride::spontMinis,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, spontMinis))
        .def_readonly("synapse_configure",
                      &SimulationConfig::ConnectionOverride::synapseConfigure,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, synapseConfigure))
        .def_readonly("modoverride",
                      &SimulationConfig::ConnectionOverride::modoverride,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, modoverride))
        .def_readonly("synapse_delay_override",
                      &SimulationConfig::ConnectionOverride::synapseDelayOverride,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, synapseDelayOverride))
        .def_readonly("delay",
                      &SimulationConfig::ConnectionOverride::delay,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, delay))
        .def_readonly("neuromodulation_dtc",
                      &SimulationConfig::ConnectionOverride::neuromodulationDtc,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, neuromodulationDtc))
        .def_readonly("neuromodulation_strength",
                      &SimulationConfig::ConnectionOverride::neuromodulationStrength,
                      DOC_SIMULATIONCONFIG(ConnectionOverride, neuromodulationStrength));

    simConf.def(py::init<const std::string&, const std::string&>())
        .def_static(
            "from_file",
            [](py::object path) { return SimulationConfig::fromFile(py::str(path)); },
            DOC_SIMULATIONCONFIG(fromFile))
        .def_property_readonly("base_path",
                               &SimulationConfig::getBasePath,
                               DOC_SIMULATIONCONFIG(getBasePath))
        .def_property_readonly("expanded_json",
                               &SimulationConfig::getExpandedJSON,
                               DOC_SIMULATIONCONFIG(getExpandedJSON))
        .def_property_readonly("run", &SimulationConfig::getRun, DOC_SIMULATIONCONFIG(getRun))
        .def_property_readonly("output",
                               &SimulationConfig::getOutput,
                               DOC_SIMULATIONCONFIG(getOutput))
        .def_property_readonly("conditions",
                               &SimulationConfig::getConditions,
                               DOC_SIMULATIONCONFIG(getConditions))
        .def_property_readonly("network",
                               &SimulationConfig::getNetwork,
                               DOC_SIMULATIONCONFIG(getNetwork))
        .def_property_readonly("target_simulator",
                               &SimulationConfig::getTargetSimulator,
                               DOC_SIMULATIONCONFIG(getTargetSimulator))
        .def_property_readonly("node_sets_file",
                               &SimulationConfig::getNodeSetsFile,
                               DOC_SIMULATIONCONFIG(getNodeSetsFile))
        .def_property_readonly("node_set",
                               &SimulationConfig::getNodeSet,
                               DOC_SIMULATIONCONFIG(getNodeSet))
        .def_property_readonly("list_report_names",
                               &SimulationConfig::listReportNames,
                               DOC_SIMULATIONCONFIG(listReportNames))
        .def_property_readonly("list_input_names",
                               &SimulationConfig::listInputNames,
                               DOC_SIMULATIONCONFIG(listInputNames))
        .def("report", &SimulationConfig::getReport, "name"_a, DOC_SIMULATIONCONFIG(getReport))
        .def("input", &SimulationConfig::getInput, "name"_a, DOC_SIMULATIONCONFIG(getInput))
        .def("connection_overrides",
             &SimulationConfig::getConnectionOverrides,
             DOC_SIMULATIONCONFIG(getConnectionOverrides))
        .def_property_readonly("metadata",
                               &SimulationConfig::getMetaData,
                               DOC_SIMULATIONCONFIG(getMetaData))
        .def_property_readonly("beta_features",
                               &SimulationConfig::getBetaFeatures,
                               DOC_SIMULATIONCONFIG(getBetaFeatures));

    py::enum_<SimulationConfig::SimulatorType>(simConf, "SimulatorType", "SimulatorType Enum")
        .value("NEURON", SimulationConfig::SimulatorType::NEURON)
        .value("CORENEURON", SimulationConfig::SimulatorType::CORENEURON);

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
            [](EdgePopulation& obj, const std::vector<NodeID>& target) {
                return obj.afferentEdges(target);
            },
            "target"_a,
            DOC_POP_EDGE(afferentEdges))
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, NodeID target) {
                return obj.afferentEdges(std::vector<NodeID>{target});
            },
            "target"_a,
            DOC_POP_EDGE(afferentEdges))
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& source) {
                return obj.efferentEdges(source);
            },
            "source"_a,
            DOC_POP_EDGE(efferentEdges))
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, NodeID source) {
                return obj.efferentEdges(std::vector<NodeID>{source});
            },
            "source"_a,
            DOC_POP_EDGE(efferentEdges))
        .def(
            "connecting_edges",
            [](EdgePopulation& obj,
               const std::vector<NodeID>& source,
               const std::vector<NodeID>& target) { return obj.connectingEdges(source, target); },
            "source"_a,
            "target"_a,
            DOC_POP_EDGE(connectingEdges))
        .def(
            "connecting_edges",
            [](EdgePopulation& obj, NodeID source, NodeID target) {
                return obj.connectingEdges(std::vector<NodeID>{source},
                                           std::vector<NodeID>{target});
            },
            "source"_a,
            "target"_a,
            DOC_POP_EDGE(connectingEdges))
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
             DOC_SPIKEREADER_POP(get),
             "node_ids"_a = nonstd::nullopt,
             "tstart"_a = nonstd::nullopt,
             "tstop"_a = nonstd::nullopt)
        .def(
            "get_dict",
            [](const SpikeReader::Population& self,
               const py::object& node_ids = py::none(),
               const py::object& tstart = py::none(),
               const py::object& tstop = py::none()) {
                const SpikeTimes& spikes =
                    (node_ids.is_none() && tstart.is_none() && tstop.is_none())
                        ? self.getRawArrays()
                        : self.getArrays(node_ids.is_none()
                                             ? nonstd::nullopt
                                             : node_ids.cast<nonstd::optional<Selection>>(),
                                         tstart.is_none() ? nonstd::nullopt
                                                          : tstart.cast<nonstd::optional<double>>(),
                                         tstop.is_none() ? nonstd::nullopt
                                                         : tstop.cast<nonstd::optional<double>>());

                py::dict result;
                result["node_ids"] = py::array_t<NodeID>(spikes.node_ids.size(),
                                                         spikes.node_ids.data());
                result["timestamps"] = py::array_t<double>(spikes.timestamps.size(),
                                                           spikes.timestamps.data());
                return result;
            },
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
                               DOC_SPIKEREADER_POP(getTimes))
        .def_property_readonly("time_units",
                               &SpikeReader::Population::getTimeUnits,
                               DOC_REPORTREADER_POP(getTimeUnits));
    py::class_<SpikeReader>(m, "SpikeReader", DOC(bbp, sonata, SpikeReader))
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
