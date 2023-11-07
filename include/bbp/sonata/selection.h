#pragma once

#include "common.h"

#include <cstdint>
#include <utility>  // std::move
#include <vector>

namespace bbp {
namespace sonata {

class SONATA_API Selection
{
  public:
    using Value = uint64_t;
    using Values = std::vector<Value>;
    using Range = std::pair<Value, Value>;
    using Ranges = std::vector<Range>;

    Selection(Ranges ranges);

    template <typename Iterator>
    static Selection fromValues(Iterator first, Iterator last);
    static Selection fromValues(const Values& values);

    /**
     * Get a list of ranges constituting Selection
     */
    const Ranges& ranges() const;

    /**
     * Array of IDs constituting Selection
     */
    Values flatten() const;

    /**
     * Total number of elements constituting Selection
     */
    size_t flatSize() const;

    bool empty() const;

  private:
    Ranges ranges_;
};

bool SONATA_API operator==(const Selection&, const Selection&);
bool SONATA_API operator!=(const Selection&, const Selection&);

Selection SONATA_API operator&(const Selection&, const Selection&);
Selection SONATA_API operator|(const Selection&, const Selection&);

template <typename Iterator>
Selection Selection::fromValues(Iterator first, Iterator last) {
    Selection::Ranges ranges;

    Selection::Range range{0, 0};
    while (first != last) {
        const auto v = *first;
        if (v == range.second) {
            ++range.second;
        } else {
            if (range.first < range.second) {
                ranges.push_back(range);
            }
            range.first = v;
            range.second = v + 1;
        }
        ++first;
    }

    if (range.first < range.second) {
        ranges.push_back(range);
    }

    return Selection(std::move(ranges));
}

}  // namespace sonata
}  // namespace bbp
