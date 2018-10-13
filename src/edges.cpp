#include <bbp/sonata/common.h>
#include <bbp/sonata/edges.h>

#include <fmt/format.h>
#include <highfive/H5File.hpp>

#include <algorithm>


namespace bbp {
namespace sonata {

namespace {

std::set<std::string> _listChildren(const HighFive::Group& group)
{
    std::set<std::string> result;
    for (const auto& name : group.listObjectNames()) {
        result.insert(name);
    }
    return result;
}


void _checkRanges(const EdgeSelection::Ranges& ranges)
{
    for (const auto& range : ranges) {
        if (range.first >= range.second) {
            throw SonataError(fmt::format("Invalid range: {}-{}", range.first, range.second));
        }
    }
}


template<typename Iterator>
EdgeSelection _selectionFromValues(Iterator first, Iterator last)
{
    EdgeSelection::Ranges ranges;

    EdgeSelection::Range range{ 0, 0 };
    for (Iterator it = first; it != last; ++it) {
        if (*it == range.second) {
            ++range.second;
        } else {
            if (range.first < range.second) {
                ranges.push_back(range);
            }
            range.first = *it;
            range.second = *it + 1;
        }
    }

    if (range.first < range.second) {
        ranges.push_back(range);
    }

    return EdgeSelection(std::move(ranges));
}

} // unnamed namespace


EdgeSelection::EdgeSelection(EdgeSelection::Ranges&& ranges)
    : ranges_(std::move(ranges))
{
    _checkRanges(ranges_);
}


EdgeSelection::EdgeSelection(const EdgeSelection::Ranges& ranges)
    : ranges_(ranges)
{
    _checkRanges(ranges_);
}


EdgeSelection EdgeSelection::fromValues(const EdgeSelection::Values& values)
{
    return _selectionFromValues(values.begin(), values.end());
}


const EdgeSelection::Ranges& EdgeSelection::ranges() const
{
    return ranges_;
}


EdgeSelection::Values EdgeSelection::flatten() const
{
    EdgeSelection::Values result;
    result.reserve(flatSize());
    for (const auto& range : ranges_) {
        for (auto v = range.first; v < range.second; ++v) {
            result.emplace_back(v);
        }
    }
    return result;
}

size_t EdgeSelection::flatSize() const
{
    size_t result = 0;
    for (const auto& range : ranges_) {
        result += (range.second - range.first);
    }
    return result;
}


bool EdgeSelection::empty() const
{
    return ranges_.empty();
}



struct EdgeStorage::Impl
{
    Impl(const std::string& _h5FilePath, const std::string& _csvFilePath)
        : h5FilePath(_h5FilePath)
        , csvFilePath(_csvFilePath)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup("/edges"))
    {
        if (!csvFilePath.empty()) {
            throw SonataError("CSV not supported at the moment");
        }
    }

    const std::string h5FilePath;
    const std::string csvFilePath;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
};


EdgeStorage::EdgeStorage(const std::string& h5FilePath, const std::string& csvFilePath)
    : impl_(new EdgeStorage::Impl(h5FilePath, csvFilePath))
{
}


EdgeStorage::~EdgeStorage() = default;


std::set<std::string> EdgeStorage::populationNames() const
{
    std::set<std::string> result;
    for (const auto& name : impl_->h5Root.listObjectNames()) {
        result.insert(name);
    }
    return result;
}


std::shared_ptr<EdgePopulation> EdgeStorage::openPopulation(const std::string& name) const
{
    if (!impl_->h5Root.exist(name)) {
        throw SonataError(fmt::format("No such population: '{}'", name));
    }
    return std::make_shared<EdgePopulation>(impl_->h5FilePath, impl_->csvFilePath, name);
}


struct EdgePopulation::Impl
{
    Impl(const std::string& h5FilePath, const std::string&, const std::string& _name)
        : name(_name)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup("/edges").getGroup(name))
        , attributeNames(_listChildren(h5Root.getGroup("0")))
    {
        size_t groupID = 0;
        while (h5Root.exist(std::to_string(groupID))) {
            ++groupID;
        }
        if (groupID != 1) {
            throw SonataError("Only single-group populations are supported at the moment");
        }
    }

    const std::string name;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
    const std::set<std::string> attributeNames;
};


EdgePopulation::EdgePopulation(const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name)
    : impl_(new EdgePopulation::Impl(h5FilePath, csvFilePath, name))
{
}


EdgePopulation::~EdgePopulation() = default;


std::string EdgePopulation::name() const
{
    return impl_->name;
}


uint64_t EdgePopulation::size() const
{
    const auto dset = impl_->h5Root.getDataSet("edge_type_id");
    return dset.getSpace().getDimensions()[0];
}


std::string EdgePopulation::sourcePopulation() const
{
    std::string result;
    impl_->h5Root.getDataSet("source_node_id").getAttribute("node_population").read(result);
    return result;
}


std::string EdgePopulation::targetPopulation() const
{
    std::string result;
    impl_->h5Root.getDataSet("target_node_id").getAttribute("node_population").read(result);
    return result;
}


namespace {

template<typename T>
std::vector<T> _readChunk(const HighFive::DataSet& dset, const EdgeSelection::Range& range)
{
    std::vector<T> result;
    assert (range.first < range.second);
    size_t chunkSize = range.second - range.first;
    dset.select({range.first}, {chunkSize}).read(result);
    return result;
}


template<typename T, typename std::enable_if<!std::is_pod<T>::value>::type* = nullptr>
std::vector<T> _readSelection(const HighFive::DataSet& dset, const EdgeSelection& selection)
{
    if (selection.ranges().size() == 1) {
        return _readChunk<T>(dset, selection.ranges().front());
    }

    std::vector<T> result;

    // for POD types we can pre-allocate result vector... see below template specialization
    for (const auto& range: selection.ranges()) {
        for (auto& x: _readChunk<T>(dset, range)) {
            result.emplace_back(std::move(x));
        }
    }

    return result;
}


template<typename T, typename std::enable_if<std::is_pod<T>::value>::type* = nullptr>
std::vector<T> _readSelection(const HighFive::DataSet& dset, const EdgeSelection& selection)
{
    std::vector<T> result(selection.flatSize());

    T* dst = result.data();
    for (const auto& range: selection.ranges()) {
        assert (range.first < range.second);
        size_t chunkSize = range.second - range.first;
        dset.select({range.first}, {chunkSize}).read(dst);
        dst += chunkSize;
    }

    return result;
}

}  // unnamed namespace


std::vector<NodeID> EdgePopulation::sourceNodeIDs(const EdgeSelection& selection) const
{
    const auto dset = impl_->h5Root.getDataSet("source_node_id");
    return _readSelection<NodeID>(dset, selection);
}


std::vector<NodeID> EdgePopulation::targetNodeIDs(const EdgeSelection& selection) const
{
    const auto dset = impl_->h5Root.getDataSet("target_node_id");
    return _readSelection<NodeID>(dset, selection);
}


const std::set<std::string>& EdgePopulation::attributeNames() const
{
    return impl_->attributeNames;
}


template<typename T>
std::vector<T> EdgePopulation::getAttribute(const std::string& name, const EdgeSelection& selection) const
{
    if (attributeNames().count(name) == 0) {
        throw SonataError(fmt::format("No such attribute: '{}'", name));
    }
    const auto dset = impl_->h5Root.getGroup("0").getDataSet(name);
    return _readSelection<T>(dset, selection);
}


template<typename T>
std::vector<T> EdgePopulation::getAttribute(const std::string& name, const EdgeSelection& selection, const T&) const
{
    // with single-group populations default value is not actually used
    return getAttribute<T>(name, selection);
}


std::string EdgePopulation::_attributeDataType(const std::string& name)
{
    if (attributeNames().count(name) == 0) {
        throw SonataError(fmt::format("No such attribute: '{}'", name));
    }
    const auto dtype = impl_->h5Root.getGroup("0").getDataSet(name).getDataType();
    if (dtype == HighFive::AtomicType<int8_t>()) {
        return "int8_t";
    } else
    if (dtype == HighFive::AtomicType<uint8_t>()) {
        return "uint8_t";
    } else
    if (dtype == HighFive::AtomicType<int16_t>()) {
        return "int16_t";
    } else
    if (dtype == HighFive::AtomicType<uint16_t>()) {
        return "uint16_t";
    } else
    if (dtype == HighFive::AtomicType<int32_t>()) {
        return "int32_t";
    } else
    if (dtype == HighFive::AtomicType<uint32_t>()) {
        return "uint32_t";
    } else
    if (dtype == HighFive::AtomicType<int64_t>()) {
        return "int64_t";
    } else
    if (dtype == HighFive::AtomicType<uint64_t>()) {
        return "uint64_t";
    } else
    if (dtype == HighFive::AtomicType<float>()) {
        return "float";
    } else
    if (dtype == HighFive::AtomicType<double>()) {
        return "double";
    } else
    if (dtype == HighFive::AtomicType<std::string>()) {
        return "string";
    } else {
        throw SonataError("Unexpected datatype");
    }
}


namespace {

EdgeSelection _resolveIndex(const HighFive::Group& indexGroup, const NodeID nodeID)
{
    typedef std::vector<std::vector<uint64_t>> RawIndex;

    RawIndex primaryRange;
    indexGroup
        .getDataSet("node_id_to_ranges")
        .select({ nodeID, 0 }, { 1, 2 })
        .read(primaryRange);

    const uint64_t primaryRangeBegin = primaryRange[0][0];
    const uint64_t primaryRangeEnd = primaryRange[0][1];

    if (primaryRangeBegin >= primaryRangeEnd) {
        return EdgeSelection({});
    }

    RawIndex secondaryRange;
    indexGroup
        .getDataSet("range_to_edge_id")
        .select({ primaryRangeBegin, 0 }, { primaryRangeEnd - primaryRangeBegin, 2 })
        .read(secondaryRange);

    EdgeSelection::Ranges ranges;
    ranges.reserve(secondaryRange.size());

    for (const auto& row: secondaryRange) {
        ranges.emplace_back(row[0], row[1]);
    }

    return EdgeSelection(std::move(ranges));
}


EdgeSelection _resolveIndex(const HighFive::Group& indexGroup, const std::vector<NodeID>& nodeIDs)
{
    if (nodeIDs.size() == 1) {
        return _resolveIndex(indexGroup, nodeIDs[0]);
    }
    // TODO optimize: bulk read for primary index
    // TODO optimize: range merging
    std::set<EdgeID> result;
    for (NodeID nodeID : nodeIDs) {
        const auto ids = _resolveIndex(indexGroup, nodeID).flatten();
        result.insert(ids.begin(), ids.end());
    }
    return _selectionFromValues(result.begin(), result.end());
}

} // unnamed namespace


EdgeSelection EdgePopulation::afferentEdges(const std::vector<NodeID>& target) const
{
    const auto& indexGroup = impl_->h5Root.getGroup("indices/target_to_source");
    return _resolveIndex(indexGroup, target);
}


EdgeSelection EdgePopulation::efferentEdges(const std::vector<NodeID>& source) const
{
    const auto& indexGroup = impl_->h5Root.getGroup("indices/source_to_target");
    return _resolveIndex(indexGroup, source);
}


EdgeSelection EdgePopulation::connectingEdges(const std::vector<NodeID>& source, const std::vector<NodeID>& target) const
{
    // TODO optimize: range intersection
    const auto pre = efferentEdges(source).flatten();
    const auto post = afferentEdges(target).flatten();
    assert(std::is_sorted(pre.begin(), pre.end()));  // return result of _resolveIndex is sorted
    assert(std::is_sorted(post.begin(), post.end()));
    EdgeSelection::Values result;
    set_intersection(
        pre.begin(), pre.end(),
        post.begin(), post.end(),
        std::back_inserter(result));
    return _selectionFromValues(result.begin(), result.end());
}


#define INSTANTIATE_TEMPLATE_METHODS(T) \
    template std::vector<T> EdgePopulation::getAttribute<T>( \
        const std::string&, const EdgeSelection&) const; \
    template std::vector<T> EdgePopulation::getAttribute<T>( \
        const std::string&, const EdgeSelection&, const T&) const; \


INSTANTIATE_TEMPLATE_METHODS(float)
INSTANTIATE_TEMPLATE_METHODS(double)

INSTANTIATE_TEMPLATE_METHODS(int8_t)
INSTANTIATE_TEMPLATE_METHODS(uint8_t)
INSTANTIATE_TEMPLATE_METHODS(int16_t)
INSTANTIATE_TEMPLATE_METHODS(uint16_t)
INSTANTIATE_TEMPLATE_METHODS(int32_t)
INSTANTIATE_TEMPLATE_METHODS(uint32_t)
INSTANTIATE_TEMPLATE_METHODS(int64_t)
INSTANTIATE_TEMPLATE_METHODS(uint64_t)

INSTANTIATE_TEMPLATE_METHODS(std::string)

#undef INSTANTIATE_TEMPLATE_METHODS

}
} // namespace bbp::sonata
