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
        .def(
            "ranges",
            &EdgeSelection::ranges,
            "Get a list of ranges constituting EdgeSelection"
        )
        .def(
            "flatten",
            &EdgeSelection::flatten,
            "List of edge IDs constituting EdgeSelection"
        )
        .def(
            "flat_size",
            &EdgeSelection::flatSize,
            "Total number of edges constituting EdgeSelection"
        )
        ;

    py::class_<EdgePopulation, std::shared_ptr<EdgePopulation>>(
        m, "EdgePopulation", "Collection of edges with attributes and connectivity index"
    )
        .def(py::init<const std::string&, const std::string&, const std::string&>())
        .def(
            "name",
            &EdgePopulation::name,
            "Population name"
        )
        .def(
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
        .def(
            "attribute_names",
            &EdgePopulation::attributeNames,
            "Set of edge attribute names"
        )
        .def(
            "get_attribute",
            [](EdgePopulation& obj, const std::string& name, EdgeID edgeID) {
                const auto selection = EdgeSelection::fromValues({edgeID});
                const auto dtype = obj._attributeDataType(name);
                if (dtype == "int8_t") {
                    return py::cast(obj.getAttribute<int8_t>(name, selection)[0]);
                } else
                if (dtype == "uint8_t") {
                    return py::cast(obj.getAttribute<uint8_t>(name, selection)[0]);
                }  else
                if (dtype == "int16_t") {
                    return py::cast(obj.getAttribute<int16_t>(name, selection)[0]);
                } else
                if (dtype == "uint16_t") {
                    return py::cast(obj.getAttribute<uint16_t>(name, selection)[0]);
                } else
                if (dtype == "int32_t") {
                    return py::cast(obj.getAttribute<int32_t>(name, selection)[0]);
                } else
                if (dtype == "uint32_t") {
                    return py::cast(obj.getAttribute<uint32_t>(name, selection)[0]);
                } else
                if (dtype == "int64_t") {
                    return py::cast(obj.getAttribute<int64_t>(name, selection)[0]);
                } else
                if (dtype == "uint64_t") {
                    return py::cast(obj.getAttribute<uint64_t>(name, selection)[0]);
                } else
                if (dtype == "float") {
                    return py::cast(obj.getAttribute<float>(name, selection)[0]);
                } else
                if (dtype == "double") {
                    return py::cast(obj.getAttribute<double>(name, selection)[0]);
                } else if (dtype == "string") {
                    return py::cast(obj.getAttribute<std::string>(name, selection)[0]);
                } else {
                    throw SonataError(std::string("Unexpected dtype: ") + dtype);
                }
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
                if (dtype == "int8_t") {
                    return asArray(obj.getAttribute<int8_t>(name, selection));
                } else
                if (dtype == "uint8_t") {
                    return asArray(obj.getAttribute<uint8_t>(name, selection));
                }  else
                if (dtype == "int16_t") {
                    return asArray(obj.getAttribute<int16_t>(name, selection));
                } else
                if (dtype == "uint16_t") {
                    return asArray(obj.getAttribute<uint16_t>(name, selection));
                } else
                if (dtype == "int32_t") {
                    return asArray(obj.getAttribute<int32_t>(name, selection));
                } else
                if (dtype == "uint32_t") {
                    return asArray(obj.getAttribute<uint32_t>(name, selection));
                } else
                if (dtype == "int64_t") {
                    return asArray(obj.getAttribute<int64_t>(name, selection));
                } else
                if (dtype == "uint64_t") {
                    return asArray(obj.getAttribute<uint64_t>(name, selection));
                } else
                if (dtype == "float") {
                    return asArray(obj.getAttribute<float>(name, selection));
                } else
                if (dtype == "double") {
                    return asArray(obj.getAttribute<double>(name, selection));
                } else if (dtype == "string") {
                    return asArray(obj.getAttribute<std::string>(name, selection));
                } else {
                    throw SonataError(std::string("Unexpected dtype: ") + dtype);
                }
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
                if (dtype == "int8_t") {
                    return asArray(obj.getAttribute<int8_t>(name, selection, defaultValue.cast<int8_t>()));
                } else
                if (dtype == "uint8_t") {
                    return asArray(obj.getAttribute<uint8_t>(name, selection, defaultValue.cast<uint8_t>()));
                }  else
                if (dtype == "int16_t") {
                    return asArray(obj.getAttribute<int16_t>(name, selection, defaultValue.cast<int16_t>()));
                } else
                if (dtype == "uint16_t") {
                    return asArray(obj.getAttribute<uint16_t>(name, selection, defaultValue.cast<uint16_t>()));
                } else
                if (dtype == "int32_t") {
                    return asArray(obj.getAttribute<int32_t>(name, selection, defaultValue.cast<int32_t>()));
                } else
                if (dtype == "uint32_t") {
                    return asArray(obj.getAttribute<uint32_t>(name, selection, defaultValue.cast<uint32_t>()));
                } else
                if (dtype == "int64_t") {
                    return asArray(obj.getAttribute<int64_t>(name, selection, defaultValue.cast<int64_t>()));
                } else
                if (dtype == "uint64_t") {
                    return asArray(obj.getAttribute<uint64_t>(name, selection, defaultValue.cast<uint64_t>()));
                } else
                if (dtype == "float") {
                    return asArray(obj.getAttribute<float>(name, selection, defaultValue.cast<float>()));
                } else
                if (dtype == "double") {
                    return asArray(obj.getAttribute<double>(name, selection, defaultValue.cast<double>()));
                } else if (dtype == "string") {
                    return asArray(obj.getAttribute<std::string>(name, selection, defaultValue.cast<std::string>()));
                } else {
                    throw SonataError(std::string("Unexpected dtype: ") + dtype);
                }
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
        .def(
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
