#include <bbp/sonata/selection.h>

#include <fmt/format.h>

#include "read_bulk.hpp"

namespace bbp {
namespace sonata {

namespace detail {
using Range = Selection::Range;
using Ranges = Selection::Ranges;

void _checkRanges(const Ranges& ranges) {
    for (const auto& range : ranges) {
        if (range.first >= range.second) {
            throw SonataError(fmt::format("Invalid range: {}-{}", range.first, range.second));
        }
    }
}

Ranges _sortAndMerge(const Ranges& ranges) {
    return bulk_read::sortAndMerge(ranges);
}

Selection intersection_(const Ranges& lhs, const Ranges& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return Selection({});
    }
    Ranges r0 = detail::_sortAndMerge(lhs);
    Ranges r1 = detail::_sortAndMerge(rhs);

    auto it0 = r0.cbegin();
    auto it1 = r1.cbegin();

    Ranges ret;
    while (it0 != r0.cend() && it1 != r1.cend()) {
        auto start = std::max(it0->first, it1->first);
        auto end = std::min(it0->second, it1->second);
        if (start < end) {
            ret.emplace_back(start, end);
        }

        if (it0->second < it1->second) {
            ++it0;
        } else {
            ++it1;
        }
    }

    return Selection(std::move(ret));
}

Selection union_(const Ranges& lhs, const Ranges& rhs) {
    Ranges ret;
    std::copy(lhs.begin(), lhs.end(), std::back_inserter(ret));
    std::copy(rhs.begin(), rhs.end(), std::back_inserter(ret));
    ret = detail::_sortAndMerge(ret);
    return Selection(std::move(ret));
}
}  // namespace detail


Selection::Selection(Selection::Ranges ranges)
    : ranges_(std::move(ranges)) {
    detail::_checkRanges(ranges_);
}


Selection Selection::fromValues(const Selection::Values& values) {
    return fromValues(values.begin(), values.end());
}


const Selection::Ranges& Selection::ranges() const {
    return ranges_;
}


Selection::Values Selection::flatten() const {
    return bulk_read::detail::flattenRanges<Selection::Value>(ranges_);
}


size_t Selection::flatSize() const {
    return bulk_read::detail::flatSize(ranges_);
}


bool Selection::empty() const {
    return ranges().empty();
}


bool operator==(const Selection& lhs, const Selection& rhs) {
    return lhs.ranges() == rhs.ranges();
}


bool operator!=(const Selection& lhs, const Selection& rhs) {
    return !(lhs == rhs);
}


Selection operator&(const Selection& lhs, const Selection& rhs) {
    return detail::intersection_(lhs.ranges(), rhs.ranges());
}


Selection operator|(const Selection& lhs, const Selection& rhs) {
    return detail::union_(lhs.ranges(), rhs.ranges());
}


}  // namespace sonata
}  // namespace bbp
