#pragma once

#include <vector>

#include <highfive/H5File.hpp>

namespace bbp {
namespace sonata {
namespace detail {


template <class Range>
HighFive::HyperSlab make_hyperslab(const std::vector<Range>& ranges) {
    HighFive::HyperSlab slab;
    for (const auto& range : ranges) {
        size_t i_begin = std::get<0>(range);
        size_t i_end = std::get<1>(range);
        slab |= HighFive::RegularHyperSlab({i_begin}, {i_end - i_begin});
    }

    return slab;
}

template <class T>
std::vector<T> readCanonicalSelection(const HighFive::DataSet& dset, const Selection& selection) {
    if (selection.empty()) {
        return {};
    }

    return dset.select(make_hyperslab(selection.ranges())).template read<std::vector<T>>();
}

}  // namespace detail
}  // namespace sonata
}  // namespace bbp
