#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>

#include <bbp/sonata/population.h>

#define SONATA_PAGESIZE (4 * 1 << 20)

namespace bbp {
namespace sonata {
namespace bulk_read {

namespace detail {

/** Is the selection sorted and non-overlapping?
 */
template <class Range>
bool isCanonical(const std::vector<Range>& ranges) {
    for (size_t i = 1; i < ranges.size(); ++i) {
        if (std::get<1>(ranges[i - 1]) > std::get<0>(ranges[i])) {
            return false;
        }
    }

    return true;
}

inline bool isCanonical(const Selection selection) {
    return isCanonical(selection.ranges());
}

/** Number of elements in the selection.
 */
template <class Range>
size_t flatSize(const std::vector<Range>& ranges) {
    size_t size = 0;
    for (const auto& range : ranges) {
        size += std::get<1>(range) - std::get<0>(range);
    }

    return size;
}

template <class T, class Pred>
void erase_if(std::vector<T>& v, Pred pred) {
    auto it = std::remove_if(v.begin(), v.end(), pred);
    v.erase(it, v.end());
}
}  // namespace detail

/** Sort the selection and merge small gaps.
 *
 * The ranges of the selection are sorted and then ranges that are separated by
 * a gap of less than `min_gap_size` are merged into a single range. A gap size
 * of: `0` will only sort, `1` will merge ranges that touch, pass the page size
 * to merge anything that's separated less than one page.
 *
 * Reading only a few elements from each page could lead to reading very large
 * ranges into memory. To avoid this issue the one must pick a maximum
 * aggregated block size. Consecutive blocks wont be merged if the current
 * block exceeds the threshold `max_aggregated_block_size`.
 *
 * Note, that the returned selection is canonical (sorted and non-overlapping)
 * and may have a larger `flatSize` than `ranges`. Additionally any empty
 * ranges are removed.
 */
template <class Range>
std::vector<Range> sortAndMerge(const std::vector<Range>& ranges,
                                size_t min_gap_size = 1,
                                size_t max_aggregated_block_size = size_t(-1)) {
    if (ranges.empty()) {
        return std::vector<Range>{};
    }

    std::vector<Range> ret;
    std::vector<Range> sorted(ranges);
    std::sort(sorted.begin(), sorted.end());

    detail::erase_if(sorted,
                     [](const auto& range) { return std::get<0>(range) >= std::get<1>(range); });

    if (sorted.empty()) {
        return std::vector<Range>{};
    }

    auto it = sorted.cbegin();
    ret.push_back(*(it++));

    for (; it != sorted.cend(); ++it) {
        auto& current_range = ret.back();
        auto& last = std::get<1>(current_range);

        size_t current_range_size = last - std::get<0>(current_range);
        if (last + min_gap_size <= std::get<0>(*it) ||
            current_range_size >= max_aggregated_block_size) {
            // Start a new range.
            ret.push_back(*it);
        } else {
            // Extend the current range.
            last = std::max(last, std::get<1>(*it));
        }
    }

    return ret;
}

inline Selection sortAndMerge(const Selection& selection, size_t min_gap_size = 0) {
    return Selection(sortAndMerge(selection.ranges(), min_gap_size));
}


/** Extract a slice of values from a block.
 *
 * This is one step of the merge-read-extract algorithm. It operates on one
 * block of data, i.e. one merged range.
 *
 * Remember ranges refer to the indices of the array on disk. They are not the
 * indices into either `out` or `buffer`.
 *
 * This copies the slice `range_small` from `buffer` to `out`. The `buffer` has
 * size `range_large[1] - range_large[0]`. The `out` pointer must point to the
 * first element to be written to.
 */
template <class T, class Range>
void extractBlock(T* const out,
                  T const* const buffer,
                  const Range& range_large,
                  const Range& range_small) {
    size_t i0_small = std::get<0>(range_small);
    size_t i1_small = std::get<1>(range_small);

    size_t i0_large = std::get<0>(range_large);

    size_t buffer_offset = i0_small - i0_large;
    size_t count_small = i1_small - i0_small;

    std::copy(buffer + buffer_offset, buffer + buffer_offset + count_small, out);
}

/** Read larger block and extract required values in memory.
 *
 *  This implements the read and extract parts of the merge-read-extract
 *  algorithm.
 *
 *  The ranges denote the indices of the array on disk. The algorithm will read
 *  one block, i.e. one element of `ranges` at a time. It reads a block by
 *  calling
 *
 *      readBlock(buffer, range);
 *
 *  the function object `readBlock` must fill `buffer` (an `std::vector<T>`)
 *  with the values for `range`. The algorithm will then extract the required
 *  values (controlled by `subranges`).
 *
 *  Note that both `ranges` and `subranges` must be canonical (sorted and
 *  non-overlapping). Additionally, any range in `subranges` must be fully
 *  contained in exactly one range in `ranges`.
 */
template <class T, class F, class Range>
std::vector<T> bulkRead(F readBlock,
                        const std::vector<Range>& ranges,
                        const std::vector<Range>& subranges) {
    std::vector<T> values(detail::flatSize(subranges));
    T* values_ptr = values.data();

    std::vector<T> buffer;

    size_t k_sub = 0;
    size_t n_sub = subranges.size();
    for (const auto& range : ranges) {
        readBlock(buffer, range);

        for (; k_sub < n_sub; ++k_sub) {
            const auto& subrange = subranges[k_sub];
            if (std::get<1>(subrange) > std::get<1>(range)) {
                break;
            }

            extractBlock(values_ptr, buffer.data(), range, subrange);
            values_ptr += std::get<1>(subrange) - std::get<0>(subrange);
        }
    }

    return values;
}

/** Read `ranges` using merge-read-extract.
 *
 *  @sa `sortAndMerge` and `bulkRead`.
 */
template <class T, class F, class Ranges>
std::vector<T> bulkRead(F readBlock,
                        const std::vector<Ranges>& ranges,
                        size_t min_gap_size,
                        size_t max_aggregated_block_size) {
    auto super_ranges = sortAndMerge(ranges, min_gap_size, max_aggregated_block_size);
    return bulkRead<T>(readBlock, super_ranges, ranges);
}

/** Read `ranges` using merge-read-extract.
 *
 *  @sa `sortAndMerge` and `bulkRead`.
 */
template <class T, class F>
std::vector<T> bulkRead(F readBlock,
                        const Selection& selection,
                        size_t min_gap_size,
                        size_t max_aggregated_block_size) {
    return bulkRead<T>(readBlock, selection.ranges(), min_gap_size, max_aggregated_block_size);
}

}  // namespace bulk_read
}  // namespace sonata
}  // namespace bbp
