#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <bbp/sonata/common.h>
#include <bbp/sonata/edges.h>
#include <bbp/sonata/nodes.h>

#include <fmt/format.h>

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
        return fmt::format(msg, fmt::arg("elem", Population::ELEMENT));
    };
    return py::class_<Population, std::shared_ptr<Population>>(m, clsName, docString)
        .def(py::init<const std::string&, const std::string&, const std::string&>())
        .def_property_readonly("name", &Population::name, "Population name")
        .def_property_readonly(
            "size",
            &Population::size,
            imbueElementName("Total number of {elem}s in the population").c_str())
        .def_property_readonly("attribute_names",
                               &Population::attributeNames,
                               "Set of attribute names")
        .def_property_readonly("enumeration_names",
                               &Population::enumerationNames,
                               "Set of enumeration names")
        .def("__len__",
             &Population::size,
             imbueElementName("Get the total number of {elem}s in the population").c_str())
        .def("select_all",
             &Population::selectAll,
             imbueElementName("Get selection of all {elem}s in the population").c_str())
        .def("enumeration_values",
             &Population::enumerationValues,
             py::arg("name"),
             "Get all allowed enumeration values for a given attribute name")
        .def(
            "get_attribute",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._attributeDataType(name, true);
                DISPATCH_TYPE(dtype, getAttribute, obj, name, selection);
            },
            py::arg("name"),
            py::arg(imbueElementName("{elem}_id").c_str()),
            imbueElementName("Get attribute value for a given {elem}.\n"
                             "Raises an exception if attribute is not defined for this {elem}.")
                .c_str())
        .def(
            "get_attribute",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._attributeDataType(name, true);
                DISPATCH_TYPE(dtype, getAttributeVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName("Get attribute values for a given {elem} selection.\n"
                             "Raises an exception if attribute is not defined for some {elem}s.")
                .c_str())
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
            imbueElementName("Get attribute values for a given {elem} selection.\n"
                             "Use default value for {elem}s where attribute is not defined\n"
                             "(it should still be one of population attributes).")
                .c_str())
        .def_property_readonly("dynamics_attribute_names",
                               &Population::dynamicsAttributeNames,
                               "Set of dynamics attribute names")
        .def(
            "get_dynamics_attribute",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._dynamicsAttributeDataType(name);
                DISPATCH_TYPE(dtype, getDynamicsAttribute, obj, name, selection);
            },
            py::arg("name"),
            py::arg(imbueElementName("{elem}_id").c_str()),
            imbueElementName("Get dynamics attribute value for a given {elem}.\n"
                             "Raises an exception if attribute is not defined for this {elem}.")
                .c_str())
        .def(
            "get_dynamics_attribute",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._dynamicsAttributeDataType(name);
                DISPATCH_TYPE(dtype, getDynamicsAttributeVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName("Get dynamics attribute values for a given {elem} selection.\n"
                             "Raises an exception if attribute is not defined for some {elem}s.")
                .c_str())
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
            imbueElementName("Get dynamics attribute values for a given {elem} selection.\n"
                             "Use default value for {elem}s where attribute is not defined\n"
                             "(it should still be one of population attributes).")
                .c_str())
        .def(
            "get_enumeration",
            [](Population& obj, const std::string& name, Selection::Value elemID) {
                const auto selection = Selection::fromValues({elemID});
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getEnumerationVector, obj, name, selection);
            },
            "name"_a,
            py::arg(imbueElementName("{elem}_id").c_str()),
            imbueElementName(
                "Get enumeration values for a given {elem} selection.\n"
                "Raises an exception if the enumeration is not defined for some {elem}s.")
                .c_str())
        .def(
            "get_enumeration",
            [](Population& obj, const std::string& name, const Selection& selection) {
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getEnumerationVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            imbueElementName(
                "Get enumeration values for a given {elem} selection.\n"
                "Raises an exception if the enumeration is not defined for some {elem}s.")
                .c_str());
}


template <typename Storage>
py::class_<Storage> bindStorageClass(py::module& m, const char* clsName, const char* popClsName) {
    return py::class_<Storage>(m,
                               clsName,
                               fmt::format("Collection of {}`s stored in H5 file (+ optional CSV)",
                                           popClsName)
                                   .c_str())
        .def(py::init<const std::string&, const std::string&>(),
             "h5_filepath"_a,
             "csv_filepath"_a = "")
        .def_property_readonly("population_names",
                               &Storage::populationNames,
                               "Set of population names")
        .def("open_population",
             &Storage::openPopulation,
             "name"_a,
             fmt::format("Get {} for a given population name", popClsName).c_str());
}
}  // unnamed namespace


PYBIND11_MODULE(libsonata, m) {
    py::class_<Selection>(m,
                          "Selection",
                          "ID sequence in the form convenient for querying attributes")
        .def(py::init<const Selection::Ranges&>(), "ranges"_a, "Selection from list of intervals")
        .def(py::init([](py::array_t<uint64_t, py::array::c_style | py::array::forcecast> values) {
                 const auto raw = values.unchecked<1>();
                 return Selection::fromValues(raw.data(0), raw.data(raw.shape(0)));
             }),
             "values"_a,
             "Selection from list of IDs")
        .def_property_readonly("ranges",
                               &Selection::ranges,
                               "Get a list of ranges constituting Selection")
        .def(
            "flatten",
            [](Selection& obj) { return asArray(obj.flatten()); },
            "Array of IDs constituting Selection")
        .def_property_readonly("flat_size",
                               &Selection::flatSize,
                               "Total number of elements constituting Selection")
        .def(
            "__bool__",
            [](const Selection& obj) { return !obj.empty(); },
            "If EdgeSelection is not empty")

        .def("__eq__", &bbp::sonata::operator==, "Compare selection contents are equal")

        .def("__ne__", &bbp::sonata::operator!=, "Compare selection contents are not equal");

    bindPopulationClass<NodePopulation>(m, "NodePopulation", "Collection of nodes with attributes")
        .def(
            "match_values",
            [](NodePopulation& obj, const std::string& name, const size_t value) {
                return obj.matchAttributeValues(name, value);
            },
            "name"_a,
            "value"_a,
            "Return selection where the attribute `name` has values matching the int `value`")
        .def(
            "match_values",
            [](NodePopulation& obj, const std::string& name, const std::string& value) {
                return obj.matchAttributeValues(name, value);
            },
            "name"_a,
            "value"_a,
            "Return selection where the attribute `name` has values matching the string `value`");

    bindStorageClass<NodeStorage>(m, "NodeStorage", "NodePopulation");

    bindPopulationClass<EdgePopulation>(
        m, "EdgePopulation", "Collection of edges with attributes and connectivity index")
        .def_property_readonly("source", &EdgePopulation::source, "Source node population")
        .def_property_readonly("target", &EdgePopulation::target, "Target node population")
        .def(
            "source_node",
            [](EdgePopulation& obj, EdgeID edgeID) {
                return obj.sourceNodeIDs(Selection::fromValues({edgeID}))[0];
            },
            "edge_id"_a,
            "Source node ID for given edge")
        .def(
            "source_nodes",
            [](EdgePopulation& obj, const Selection& selection) {
                return asArray(obj.sourceNodeIDs(selection));
            },
            "selection"_a,
            "Source node IDs for given Selection")
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
            "Source node IDs for given Selection")
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, NodeID target) {
                return obj.afferentEdges(std::vector<NodeID>{target});
            },
            "target"_a,
            "Find all edges targeting given node")
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& target) {
                return obj.afferentEdges(target);
            },
            "target"_a,
            "Find all edges targeting given nodes")
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, NodeID source) {
                return obj.efferentEdges(std::vector<NodeID>{source});
            },
            "source"_a,
            "Find all edges originating from given node")
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& source) {
                return obj.efferentEdges(source);
            },
            "source"_a,
            "Find all edges originating from given nodes")
        .def(
            "connecting_edges",
            [](EdgePopulation& obj, NodeID source, NodeID target) {
                return obj.connectingEdges(std::vector<NodeID>{source},
                                           std::vector<NodeID>{target});
            },
            "source"_a,
            "target"_a,
            "Find all edges connecting two given nodes")
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
                    "Write bidirectional node->edge indices to EdgePopulation HDF5");

    bindStorageClass<EdgeStorage>(m, "EdgeStorage", "EdgePopulation");

    py::register_exception<SonataError>(m, "SonataError");
}
