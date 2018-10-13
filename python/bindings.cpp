#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <bbp/sonata/edges.h>

#include <cstdint>
#include <memory>
#include <string>


namespace py = pybind11;

using namespace pybind11::literals;

using namespace bbp::sonata;


namespace {

template<typename T>
py::capsule freeWhenDone(T *ptr)
{
    return py::capsule(ptr, [](void* p) {
        delete reinterpret_cast<T*>(p);
    });
}


// This little beauty passes std::vector data to NumPy array without extra copy.
// For the details, please refer to:
// https://github.com/pybind/pybind11/issues/1042#issuecomment-325941022
template <typename T>
py::array asArray(std::vector<T>&& values)
{
    auto ptr = new std::vector<T>(std::move(values));
    return py::array(ptr->size(), ptr->data(), freeWhenDone(ptr));
}


// As std::string is not a POD type, we wrap each value with py::object
template <>
py::array asArray(std::vector<std::string>&& values)
{
    auto ptr = new std::vector<py::object>();
    ptr->reserve(values.size());
    for (auto& s: values) {
        ptr->push_back(py::cast(s));
    }
    return py::array(py::dtype("object"), ptr->size(), ptr->data(), freeWhenDone(ptr));
}


template<typename T>
py::object getAttribute(const EdgePopulation& obj, const std::string& name, const EdgeSelection& selection)
{
    return py::cast(obj.getAttribute<T>(name, selection)[0]);
}


template<typename T>
py::object getAttributeVector(const EdgePopulation& obj, const std::string& name, const EdgeSelection& selection)
{
    return asArray(obj.getAttribute<T>(name, selection));
}


template<typename T>
py::object getAttributeVectorWithDefault(
    const EdgePopulation& obj, const std::string& name,
    const EdgeSelection& selection, const py::object& defaultValue
)
{
    return asArray(obj.getAttribute<T>(name, selection, defaultValue.cast<T>()));
}


// Emulating generic lambdas in pre-C++14
#define DISPATCH_TYPE(dtype, func, ...) \
    if (dtype == "int8_t") { \
        return func<int8_t>(__VA_ARGS__); \
    } else if (dtype == "uint8_t") { \
        return func<uint8_t>(__VA_ARGS__); \
    } else if (dtype == "int16_t") { \
        return func<int16_t>(__VA_ARGS__); \
    } else if (dtype == "uint16_t") { \
        return func<uint16_t>(__VA_ARGS__); \
    } else if (dtype == "int32_t") { \
        return func<int32_t>(__VA_ARGS__); \
    } else if (dtype == "uint32_t") { \
        return func<uint32_t>(__VA_ARGS__); \
    } else if (dtype == "int64_t") { \
        return func<int64_t>(__VA_ARGS__); \
    } else if (dtype == "uint64_t") { \
        return func<uint64_t>(__VA_ARGS__); \
    } else if (dtype == "float") { \
        return func<float>(__VA_ARGS__); \
    } else if (dtype == "double") { \
        return func<double>(__VA_ARGS__); \
    } else if (dtype == "string") { \
        return func<std::string>(__VA_ARGS__); \
    } else { \
        throw SonataError(std::string("Unexpected dtype: ") + dtype); \
    } \

} // unnamed namespace


PYBIND11_MODULE(_sonata, m)
{
    py::class_<EdgeSelection>(
        m, "EdgeSelection", "Set of edge IDs in the form convenient for querying attributes"
    )
        .def(
            py::init<const EdgeSelection::Ranges&>(),
            "ranges"_a,
            "EdgeSelection from list of intervals"
        )
        .def(
            py::init(&EdgeSelection::fromValues),
            "values"_a,
            "EdgeSelection from list of edge IDs"
        )
        .def_property_readonly(
            "ranges",
            &EdgeSelection::ranges,
            "Get a list of ranges constituting EdgeSelection"
        )
        .def(
            "flatten",
            &EdgeSelection::flatten,
            "List of edge IDs constituting EdgeSelection"
        )
        .def_property_readonly(
            "flat_size",
            &EdgeSelection::flatSize,
            "Total number of edges constituting EdgeSelection"
        )
        .def(
            "__bool__",
            [](const EdgeSelection& obj) {
                return !obj.empty();
            },
            "If EdgeSelection is not empty"
        )
        ;

    py::class_<EdgePopulation, std::shared_ptr<EdgePopulation>>(
        m, "EdgePopulation", "Collection of edges with attributes and connectivity index"
    )
        .def(py::init<const std::string&, const std::string&, const std::string&>())
        .def_property_readonly(
            "name",
            &EdgePopulation::name,
            "Population name"
        )
        .def_property_readonly(
            "size", &EdgePopulation::size,
            "Total number of edges in the population"
        )
        .def(
            "source_node",
            [](EdgePopulation& obj, EdgeID edgeID) {
                return obj.sourceNodeIDs(EdgeSelection::fromValues({edgeID}))[0];
            },
            "edge_id"_a,
            "Source node ID for given edge"
        )
        .def(
            "source_nodes",
            [](EdgePopulation& obj, const EdgeSelection& selection) {
                return asArray(obj.sourceNodeIDs(selection));
            },
            "selection"_a,
            "Source node IDs for given EdgeSelection"
        )
        .def(
            "target_node",
            [](EdgePopulation& obj, EdgeID edgeID) {
                return obj.targetNodeIDs(EdgeSelection::fromValues({edgeID}))[0];
            },
            "edge_id"_a,
            "Target node ID for given edge"
        )
        .def(
            "target_nodes",
            [](EdgePopulation& obj, const EdgeSelection& selection) {
                return asArray(obj.targetNodeIDs(selection));
            },
            "selection"_a,
            "Source node IDs for given EdgeSelection"
        )
        .def_property_readonly(
            "attribute_names",
            &EdgePopulation::attributeNames,
            "Set of edge attribute names"
        )
        .def(
            "get_attribute",
            [](EdgePopulation& obj, const std::string& name, EdgeID edgeID) {
                const auto selection = EdgeSelection::fromValues({edgeID});
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getAttribute, obj, name, selection);
            },
            "name"_a,
            "edge_id"_a,
            "Get attribute value for a given edge.\n"
            "Raises an exception if attribute is not defined for this edge."
        )
        .def(
            "get_attribute",
            [](EdgePopulation& obj, const std::string& name, const EdgeSelection& selection) {
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getAttributeVector, obj, name, selection);
            },
            "name"_a,
            "selection"_a,
            "Get attribute values for a given edge selection.\n"
            "Raises an exception if attribute is not defined for some edges."
        )
        .def(
            "get_attribute",
            [](EdgePopulation& obj, const std::string& name, const EdgeSelection& selection, const py::object& defaultValue) {
                const auto dtype = obj._attributeDataType(name);
                DISPATCH_TYPE(dtype, getAttributeVectorWithDefault, obj, name, selection, defaultValue);
            },
            "name"_a,
            "selection"_a,
            "default_value"_a,
            "Get attribute values for a given edge selection.\n"
            "Use default value for edges where attribute is not defined\n"
            "(it should still be one of population attributes)."
        )
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, NodeID target) {
                return obj.afferentEdges(std::vector<NodeID>{target});
            },
            "target"_a,
            "Find all edges targeting given node"
        )
        .def(
            "afferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& target) {
                return obj.afferentEdges(target);
            },
            "target"_a,
            "Find all edges targeting given nodes"
        )
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, NodeID source) {
                return obj.efferentEdges(std::vector<NodeID>{source});
            },
            "source"_a,
            "Find all edges originating from given node"
        )
        .def(
            "efferent_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& source) {
                return obj.efferentEdges(source);
            },
            "source"_a,
            "Find all edges originating from given nodes"
        )
        .def(
            "connecting_edges",
            [](EdgePopulation& obj, NodeID source, NodeID target) {
                return obj.connectingEdges(std::vector<NodeID>{source}, std::vector<NodeID>{target});
            },
            "source"_a,
            "target"_a,
            "Find all edges connecting two given nodes"
        )
        .def(
            "connecting_edges",
            [](EdgePopulation& obj, const std::vector<NodeID>& source, const std::vector<NodeID>& target) {
                return obj.connectingEdges(source, target);
            },
            "source"_a,
            "target"_a,
            "Find all edges connecting two given node sets"
        )
        ;

    py::class_<EdgeStorage>(
        m, "EdgeStorage", "Collection of `EdgePopulation`s stored in H5 file (+ optional CSV)"
    )
        .def(
            py::init<const std::string&, const std::string&>(),
            "h5_filepath"_a,
            "csv_filepath"_a = ""
        )
        .def_property_readonly(
            "population_names",
            &EdgeStorage::populationNames,
            "Set of population names"
        )
        .def(
            "open_population",
            &EdgeStorage::openPopulation,
            "name"_a,
            "Get EdgePopulation for a given population name"
        )
        ;

    py::register_exception<SonataError>(m, "SonataError");
}
