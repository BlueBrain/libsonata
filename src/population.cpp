#include "population.hpp"
#include "hdf5_mutex.hpp"

#include <fmt/format.h>
#include <highfive/H5File.hpp>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

namespace {

void _checkRanges(const Selection::Ranges& ranges)
{
    for (const auto& range : ranges) {
        if (range.first >= range.second) {
            throw SonataError(fmt::format("Invalid range: {}-{}", range.first, range.second));
        }
    }
}

} // unnamed namespace


Selection::Selection(Selection::Ranges&& ranges)
    : ranges_(std::move(ranges))
{
    _checkRanges(ranges_);
}


Selection::Selection(const Selection::Ranges& ranges)
    : ranges_(ranges)
{
    _checkRanges(ranges_);
}


Selection Selection::fromValues(const Selection::Values& values)
{
    return _selectionFromValues(values.begin(), values.end());
}


const Selection::Ranges& Selection::ranges() const
{
    return ranges_;
}


Selection::Values Selection::flatten() const
{
    Selection::Values result;
    result.reserve(flatSize());
    for (const auto& range : ranges_) {
        for (auto v = range.first; v < range.second; ++v) {
            result.emplace_back(v);
        }
    }
    return result;
}


size_t Selection::flatSize() const
{
    size_t result = 0;
    for (const auto& range : ranges_) {
        result += (range.second - range.first);
    }
    return result;
}


bool Selection::empty() const
{
    return ranges_.empty();
}

//--------------------------------------------------------------------------------------------------

namespace {

std::string _getDataType(const HighFive::DataSet& dset, const std::string& name)
{
    const auto dtype = dset.getDataType();
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
        throw SonataError(fmt::format("Unexpected datatype for dataset '{}'", name));
    }
}

} // unnamed namespace


Population::Population(
    const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name, const std::string& prefix
)
    : impl_([h5FilePath, csvFilePath, name, prefix] {
        HDF5_LOCK_GUARD
        return new Population::Impl(h5FilePath, csvFilePath, name, prefix);
    }())
{
}


Population::~Population() = default;


std::string Population::name() const
{
    return impl_->name;
}


uint64_t Population::size() const
{
    HDF5_LOCK_GUARD
    const auto dset = impl_->h5Root.getDataSet(fmt::format("{}_type_id", impl_->prefix));
    return dset.getSpace().getDimensions()[0];
}


const std::set<std::string>& Population::attributeNames() const
{
    return impl_->attributeNames;
}


template<typename T>
std::vector<T> Population::getAttribute(const std::string& name, const Selection& selection) const
{
    HDF5_LOCK_GUARD
    return _readSelection<T>(impl_->getAttributeDataSet(name), selection);
}


template<typename T>
std::vector<T> Population::getAttribute(const std::string& name, const Selection& selection, const T&) const
{
    // with single-group populations default value is not actually used
    return getAttribute<T>(name, selection);
}


std::string Population::_attributeDataType(const std::string& name) const
{
    HDF5_LOCK_GUARD
    return _getDataType(impl_->getAttributeDataSet(name), name);
}


const std::set<std::string>& Population::dynamicsAttributeNames() const
{
    return impl_->dynamicsAttributeNames;
}


template <typename T>
std::vector<T> Population::getDynamicsAttribute(const std::string& name, const Selection& selection) const
{
    HDF5_LOCK_GUARD
    return _readSelection<T>(impl_->getDynamicsAttributeDataSet(name), selection);
}


template <typename T>
std::vector<T> Population::getDynamicsAttribute(const std::string& name, const Selection& selection, const T&) const
{
    // with single-group populations default value is not actually used
    return getDynamicsAttribute<T>(name, selection);
}


std::string Population::_dynamicsAttributeDataType(const std::string& name) const
{
    HDF5_LOCK_GUARD
    return _getDataType(impl_->getDynamicsAttributeDataSet(name), name);
}

//--------------------------------------------------------------------------------------------------

#define INSTANTIATE_TEMPLATE_METHODS(T) \
    template std::vector<T> Population::getAttribute<T>( \
        const std::string&, const Selection&) const; \
    template std::vector<T> Population::getAttribute<T>( \
        const std::string&, const Selection&, const T&) const; \
    template std::vector<T> Population::getDynamicsAttribute<T>( \
        const std::string&, const Selection&) const; \
    template std::vector<T> Population::getDynamicsAttribute<T>( \
        const std::string&, const Selection&, const T&) const;


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

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata
