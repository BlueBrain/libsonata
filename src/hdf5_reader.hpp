#pragma once

#include "population.hpp"
#include "read_bulk.hpp"
#include "read_canonical_selection.hpp"

namespace bbp {
namespace sonata {

namespace detail {
template <class Range>
HighFive::HyperSlab _makeHyperslab(const std::vector<Range>& ranges) {
    HighFive::HyperSlab slab;
    for (const auto& range : ranges) {
        size_t i_begin = std::get<0>(range);
        size_t i_end = std::get<1>(range);
        slab |= HighFive::RegularHyperSlab({i_begin}, {i_end - i_begin});
    }

    return slab;
}
}  // namespace detail


template <class T>
class Hdf5PluginRead1DDefault: virtual public Hdf5PluginRead1DInterface<T>
{
  public:
    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const Selection& selection) const override {
        if (selection.ranges().empty()) {
            return {};
        }

        return dset.select(detail::_makeHyperslab(selection.ranges()))
            .template read<std::vector<T>>();
    }
};

template <class T>
class Hdf5PluginRead2DDefault: virtual public Hdf5PluginRead2DInterface<T>
{
  private:
    using AltRanges = std::vector<std::array<uint64_t, 2>>;

  public:
    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const Selection& xsel,
                                 const Selection& ysel) const override {
        return readSelectionImpl(dset, xsel.ranges(), ysel.ranges());
    }

    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const AltRanges& xsel,
                                 const Selection& ysel) const override {
        return readSelectionImpl(dset, xsel, ysel.ranges());
    }

    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const Selection& xsel,
                                 const AltRanges& ysel) const override {
        return readSelectionImpl(dset, xsel.ranges(), ysel);
    }

    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const AltRanges& xsel,
                                 const AltRanges& ysel) const override {
        return readSelectionImpl(dset, xsel, ysel);
    }

  private:
    template <class XRange, class YRange>
    std::vector<T> readSelectionImpl(const HighFive::DataSet& dset,
                                     const std::vector<XRange>& xsel,
                                     const std::vector<YRange>& ysel) const {
        return detail::readCanonicalSelection<T>(dset, xsel, ysel);
    }
};

template <class T, class U>
class Hdf5PluginDefault;

template <class... Ts, class... Us>
class Hdf5PluginDefault<std::tuple<Ts...>, std::tuple<Us...>>
    : virtual public Hdf5PluginInterface<std::tuple<Ts...>, std::tuple<Us...>>,
      virtual public Hdf5PluginRead1DDefault<Ts>...,
      virtual public Hdf5PluginRead2DDefault<Us>...
{
  private:
    using AltRanges = std::vector<std::array<uint64_t, 2>>;

  public:
    HighFive::File openFile(const std::string& path) const override {
        return HighFive::File(path);
    }
};


}  // namespace sonata
}  // namespace bbp
