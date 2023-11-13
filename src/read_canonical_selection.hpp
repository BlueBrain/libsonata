#pragma once

#include "read_bulk.hpp"

namespace bbp {
namespace sonata {
namespace detail {

template <class T, class XRange, class YRange>
std::vector<T> readCanonicalSelection(const HighFive::DataSet& dset,
                                      const std::vector<XRange>& xranges,
                                      const std::vector<YRange>& yranges) {
    if (yranges.size() != 1) {
        throw SonataError("Only yranges.size() == 1 has been implemented.");
    }

    size_t j_begin = std::get<0>(yranges[0]);
    size_t j_end = std::get<1>(yranges[0]);

    constexpr size_t min_gap_size = SONATA_PAGESIZE / (2 * sizeof(uint64_t));
    constexpr size_t max_aggregated_block_size = 128 * min_gap_size;

    auto readBlock = [&](auto& buffer, const auto& xrange) {
        size_t i_begin = std::get<0>(xrange);
        size_t i_end = std::get<1>(xrange);
        dset.select({i_begin, j_begin}, {i_end - i_begin, j_end - j_begin}).read(buffer);
    };

    return bulk_read::bulkRead<T>([&readBlock](auto& buffer,
                                               const auto& range) { readBlock(buffer, range); },
                                  xranges,
                                  min_gap_size,
                                  max_aggregated_block_size);
}

}  // namespace detail
}  // namespace sonata
}  // namespace bbp
