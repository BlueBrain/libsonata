#include <bbp/sonata/report_reader.h>
#include <fmt/format.h>

#include <algorithm>      // std::copy, std::find, std::lower_bound, std::upper_bound
#include <iterator>       // std::advance, std::next
#include <unordered_set>  // std::unordered_set

constexpr double EPSILON = 1e-6;

HighFive::EnumType<bbp::sonata::SpikeReader::Population::Sorting> create_enum_sorting() {
    using bbp::sonata::SpikeReader;
    return HighFive::EnumType<SpikeReader::Population::Sorting>(
        {{"none", SpikeReader::Population::Sorting::none},
         {"by_id", SpikeReader::Population::Sorting::by_id},
         {"by_time", SpikeReader::Population::Sorting::by_time}});
}

HIGHFIVE_REGISTER_TYPE(bbp::sonata::SpikeReader::Population::Sorting, create_enum_sorting)

namespace {

using bbp::sonata::CompartmentID;
using bbp::sonata::ElementID;
using bbp::sonata::NodeID;
using bbp::sonata::Selection;
using bbp::sonata::Spike;
using bbp::sonata::Spikes;

void filterNodeIDUnsorted(Spikes& spikes, const Selection& node_ids) {
    const auto values = node_ids.flatten();
    const auto selected_values = std::unordered_set<Selection::Value>(values.begin(), values.end());
    const auto new_end =
        std::remove_if(spikes.begin(), spikes.end(), [&selected_values](const Spike& spike) {
            return selected_values.find(spike.first) == selected_values.end();
        });
    spikes.erase(new_end, spikes.end());
}

void filterNodeIDSorted(Spikes& spikes, const Selection& node_ids) {
    Spikes _spikes;
    for (const auto& range : node_ids.ranges()) {
        const auto begin = std::lower_bound(spikes.begin(),
                                            spikes.end(),
                                            std::make_pair(std::get<0>(range), 0.),
                                            [](const Spike& spike1, const Spike& spike2) {
                                                return spike1.first < spike2.first;
                                            });
        const auto end = std::upper_bound(spikes.begin(),
                                          spikes.end(),
                                          std::make_pair(std::get<1>(range) - 1, 0.),
                                          [](const Spike& spike1, const Spike& spike2) {
                                              return spike1.first < spike2.first;
                                          });

        std::move(begin, end, std::back_inserter(_spikes));
        spikes.erase(begin, end);  // have to erase, because otherwise it is no more sorted
    }
    spikes = std::move(_spikes);
}

void filterTimestampUnsorted(Spikes& spikes, double tstart, double tstop) {
    auto new_end =
        std::remove_if(spikes.begin(), spikes.end(), [&tstart, &tstop](const Spike& spike) {
            return (spike.second < tstart - EPSILON) || (spike.second > tstop + EPSILON);
        });
    spikes.erase(new_end, spikes.end());
}

void filterTimestampSorted(Spikes& spikes, double tstart, double tstop) {
    const auto end = std::upper_bound(spikes.begin(),
                                      spikes.end(),
                                      std::make_pair(0UL, tstop + EPSILON),
                                      [](const Spike& spike1, const Spike& spike2) {
                                          return spike1.second < spike2.second;
                                      });
    spikes.erase(end, spikes.end());
    const auto begin = std::lower_bound(spikes.begin(),
                                        spikes.end(),
                                        std::make_pair(0UL, tstart - EPSILON),
                                        [](const Spike& spike1, const Spike& spike2) {
                                            return spike1.second < spike2.second;
                                        });
    spikes.erase(spikes.begin(), begin);
}

inline void emplace_ids(NodeID& key, NodeID node_id, ElementID /* element_id */) {
    key = node_id;
}

inline void emplace_ids(CompartmentID& key, NodeID node_id, ElementID element_id) {
    key[0] = node_id;
    key[1] = element_id;
}

}  // anonymous namespace

namespace bbp {
namespace sonata {

SpikeReader::SpikeReader(std::string filename)
    : filename_(std::move(filename)) {}

std::vector<std::string> SpikeReader::getPopulationNames() const {
    HighFive::File file(filename_, HighFive::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikeReader::openPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{filename_, populationName});
    }

    return populations_.at(populationName);
}

std::tuple<double, double> SpikeReader::Population::getTimes() const {
    return std::tie(tstart_, tstop_);
}

Spikes SpikeReader::Population::createSpikes() const {
    Spikes spikes;
    std::transform(spike_times_.node_ids.begin(),
                   spike_times_.node_ids.end(),
                   spike_times_.timestamps.begin(),
                   std::back_inserter(spikes),
                   [](Spike::first_type node_id, Spike::second_type timestamp) {
                       return std::make_pair(node_id, timestamp);
                   });
    return spikes;
}

Spikes SpikeReader::Population::get(const nonstd::optional<Selection>& node_ids,
                                    const nonstd::optional<double>& tstart,
                                    const nonstd::optional<double>& tstop) const {
    const double start = tstart.value_or(tstart_);
    const double stop = tstop.value_or(tstop_);

    if (start < 0 - EPSILON || stop < 0 - EPSILON) {
        throw SonataError("Times cannot be negative");
    }

    if (start > stop) {
        throw SonataError("tstart should be <= to tstop");
    }

    if (node_ids and node_ids->empty()) {
        return Spikes{};
    }

    auto spikes = createSpikes();
    filterTimestamp(spikes, start, stop);

    if (node_ids) {
        filterNode(spikes, node_ids.value());
    }

    return spikes;
}

const SpikeTimes& SpikeReader::Population::getRawArrays() const {
    return spike_times_;
}

SpikeTimes SpikeReader::Population::getArrays(const nonstd::optional<Selection>& node_ids,
                                              const nonstd::optional<double>& tstart,
                                              const nonstd::optional<double>& tstop) const {
    SpikeTimes filtered_spikes;
    const auto& node_ids_selection = node_ids ? node_ids.value().flatten() : std::vector<NodeID>{};
    // Create arrays directly for required data based on conditions
    for (size_t i = 0; i < spike_times_.node_ids.size(); ++i) {
        const auto& node_id = spike_times_.node_ids[i];
        const auto& timestamp = spike_times_.timestamps[i];

        // Check if node_id is found in node_ids_selection
        bool node_ids_found = true;
        if (node_ids) {
            node_ids_found = std::find(node_ids_selection.begin(),
                                       node_ids_selection.end(),
                                       node_id) != node_ids_selection.end();
        }

        // Check if timestamp is within valid range
        bool valid_timestamp = (!tstart || timestamp >= tstart.value()) &&
                               (!tstop || timestamp <= tstop.value());

        // Include data if both conditions are satisfied
        if (node_ids_found && valid_timestamp) {
            filtered_spikes.node_ids.emplace_back(node_id);
            filtered_spikes.timestamps.emplace_back(timestamp);
        }
    }
    return filtered_spikes;
}

SpikeReader::Population::Sorting SpikeReader::Population::getSorting() const {
    return sorting_;
}

std::string SpikeReader::Population::getTimeUnits() const {
    return time_units_;
}

SpikeReader::Population::Population(const std::string& filename,
                                    const std::string& populationName) {
    HighFive::File file(filename, HighFive::File::ReadOnly);
    const auto pop_path = std::string("/spikes/") + populationName;
    const auto pop = file.getGroup(pop_path);
    auto& node_ids = spike_times_.node_ids;
    auto& timestamps = spike_times_.timestamps;

    pop.getDataSet("node_ids").read(node_ids);
    pop.getDataSet("timestamps").read(timestamps);
    pop.getDataSet("timestamps").getAttribute("units").read(time_units_);

    if (node_ids.size() != timestamps.size()) {
        throw SonataError(
            "In spikes file, 'node_ids' and 'timestamps' does not have the same size.");
    }

    if (pop.hasAttribute("sorting")) {
        pop.getAttribute("sorting").read(sorting_);
    }

    if (sorting_ == Sorting::by_time) {
        tstart_ = timestamps.empty() ? 0 : timestamps.front();
        tstop_ = timestamps.empty() ? 0 : timestamps.back();
    } else {
        tstart_ = timestamps.empty() ? 0 : *min_element(timestamps.cbegin(), timestamps.cend());
        tstop_ = timestamps.empty() ? 0 : *max_element(timestamps.cbegin(), timestamps.cend());
    }
}

void SpikeReader::Population::filterNode(Spikes& spikes, const Selection& node_ids) const {
    if (sorting_ == Sorting::by_id) {
        filterNodeIDSorted(spikes, node_ids);
    } else {
        filterNodeIDUnsorted(spikes, node_ids);
    }
}

void SpikeReader::Population::filterTimestamp(Spikes& spikes, double tstart, double tstop) const {
    if (sorting_ == Sorting::by_time) {
        filterTimestampSorted(spikes, tstart, tstop);
    } else {
        filterTimestampUnsorted(spikes, tstart, tstop);
    }
}

template <typename T>
ReportReader<T>::ReportReader(const std::string& filename)
    : file_(filename, HighFive::File::ReadOnly) {}

template <typename T>
std::vector<std::string> ReportReader<T>::getPopulationNames() const {
    return file_.getGroup("/report").listObjectNames();
}

template <typename T>
auto ReportReader<T>::openPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{file_, populationName});
    }

    return populations_.at(populationName);
}

template <typename T>
ReportReader<T>::Population::Population(const HighFive::File& file,
                                        const std::string& populationName)
    : pop_group_(file.getGroup(std::string("/report/") + populationName))
    , is_node_ids_sorted_(false) {
    const auto mapping_group = pop_group_.getGroup("mapping");
    mapping_group.getDataSet("node_ids").read(node_ids_);

    std::vector<uint64_t> index_pointers;
    mapping_group.getDataSet("index_pointers").read(index_pointers);

    if (index_pointers.size() != (node_ids_.size() + 1)) {
        throw SonataError("'index_pointers' dataset size must be 'node_ids' size plus one");
    }

    // Expand the pointers into tuples that define the range of each GID
    size_t element_ids_count = 0;
    for (size_t i = 0; i < node_ids_.size(); ++i) {
        node_ranges_.push_back({index_pointers[i], index_pointers[i + 1]});   // Range of GID
        node_offsets_.emplace_back(element_ids_count);                        // Offset in output
        node_index_.emplace_back(i);                                          // Index of previous

        element_ids_count += (index_pointers[i + 1] - index_pointers[i]);
    }
    node_offsets_.emplace_back(element_ids_count);

    {  // Sort the index according to the GIDs, if not sorted in file
        if (mapping_group.getDataSet("node_ids").hasAttribute("sorted")) {
            uint8_t sorted = 0;
            mapping_group.getDataSet("node_ids").getAttribute("sorted").read(sorted);
            is_node_ids_sorted_ = (sorted != 0);
        }

        if (!is_node_ids_sorted_) {
            // Note: The idea is to sort the positions to access the values, allowing us to
            //       maintain all vectors intact, while still being able to index the data
            std::sort(node_index_.begin(), node_index_.end(), [&](const size_t i, const size_t j) {
                return node_ids_[i] < node_ids_[j];
            });
        }
    }

    {  // Get times
        std::vector<double> times;
        mapping_group.getDataSet("time").read(times);
        tstart_ = times[0];
        tstop_ = times[1];
        tstep_ = times[2];
        mapping_group.getDataSet("time").getAttribute("units").read(time_units_);
        size_t i = 0;
        for (double t = tstart_; t < tstop_ - EPSILON; t += tstep_, ++i) {
            times_index_.emplace_back(i, t);
        }
    }

    pop_group_.getDataSet("data").getAttribute("units").read(data_units_);
}

template <typename T>
std::tuple<double, double, double> ReportReader<T>::Population::getTimes() const {
    return std::tie(tstart_, tstop_, tstep_);
}

template <typename T>
std::string ReportReader<T>::Population::getTimeUnits() const {
    return time_units_;
}

template <typename T>
std::string ReportReader<T>::Population::getDataUnits() const {
    return data_units_;
}

template <typename T>
bool ReportReader<T>::Population::getSorted() const {
    return is_node_ids_sorted_;
}

template <typename T>
std::vector<NodeID> ReportReader<T>::Population::getNodeIds() const {
    return node_ids_;
}

template <typename T>
typename ReportReader<T>::Population::NodeIdElementLayout
ReportReader<T>::Population::getNodeIdElementLayout(
    const nonstd::optional<Selection>& node_ids,
    const nonstd::optional<size_t>& _block_gap_limit) const {
    NodeIdElementLayout result;
    std::vector<NodeID> concrete_node_ids;
    size_t element_ids_count = 0;

    // Set the gap between IO blocks while fetching data (Default: 64MB / 4 x GPFS blocks)
    const size_t block_gap_limit = _block_gap_limit.value_or(16777216);

    if (block_gap_limit < 4194304) {
        throw SonataError("block_gap_limit must be at least 4194304 (16MB / 1 x GPFS block)");
    }

    // Take all nodes if no selection is provided
    if (!node_ids) {
        concrete_node_ids = node_ids_;
        result.node_ranges = node_ranges_;
        result.node_offsets = node_offsets_;
        result.node_index = node_index_;
        element_ids_count = node_offsets_.back();
    } else if (!node_ids->empty()) {
        const auto selected_node_ids = node_ids->flatten();

        for (const auto node_id : selected_node_ids) {
            const auto it = std::lower_bound(node_index_.begin(),
                                             node_index_.end(),
                                             node_id,
                                             [&](const size_t i, const NodeID node_id) {
                                                 return node_ids_[i] < node_id;
                                             });

            if (it != node_index_.end() && node_ids_[*it] == node_id) {
                const auto& range = node_ranges_[*it];

                concrete_node_ids.emplace_back(node_id);
                result.node_ranges.emplace_back(range);
                result.node_offsets.emplace_back(element_ids_count);
                result.node_index.emplace_back(result.node_index.size());

                element_ids_count += (std::get<1>(range) - std::get<0>(range));
            }
        }
    } else {
        // node_ids Selection exists, but is empty
    }

    // Extract the ElementIDs from the GIDs
    if (!concrete_node_ids.empty()) {
        // Sort the index by the selected ranges
        std::sort(result.node_index.begin(),
                  result.node_index.end(),
                  [&](const size_t i, const size_t j) {
                      return std::get<0>(result.node_ranges[i]) <
                             std::get<0>(result.node_ranges[j]);
                  });

        // Generate the {min,max} IO blocks for the requests
        size_t offset = 0;
        for (size_t i = 0; (i + 1) < result.node_index.size(); i++) {
            const auto index = result.node_index[i];
            const auto index_next = result.node_index[i + 1];
            const auto max = std::get<1>(result.node_ranges[index]);
            const auto min_next = std::get<0>(result.node_ranges[index_next]);

            if ((min_next - max) > block_gap_limit) {
                result.min_max_blocks.push_back({offset, (i + 1)});
                offset = (i + 1);
            }
        }
        result.min_max_blocks.push_back({offset, result.node_index.size()});

        // Fill the GID-ElementID mapping in blocks to reduce the file system overhead
        std::vector<ElementID> element_ids;
        auto dataset_elem_ids = pop_group_.getGroup("mapping").getDataSet("element_ids");

        result.ids.resize(element_ids_count);

        for (const auto& min_max_block : result.min_max_blocks) {
            const auto first_index = result.node_index[std::get<0>(min_max_block)];
            const auto last_index = result.node_index[std::get<1>(min_max_block) - 1];
            const auto min = std::get<0>(result.node_ranges[first_index]);
            const auto max = std::get<1>(result.node_ranges[last_index]);

            dataset_elem_ids.select({min}, {max - min}).read(element_ids);

            // Copy the values for each of the GIDs assigned into this block
            for (size_t i = std::get<0>(min_max_block); i < std::get<1>(min_max_block); ++i) {
                const auto index = result.node_index[i];
                const auto node_id = concrete_node_ids[index];
                const auto range = Selection::Range{std::get<0>(result.node_ranges[index]) - min,
                                                    std::get<1>(result.node_ranges[index]) - min};

                auto offset = result.node_offsets[index];
                for (auto i = std::get<0>(range); i < std::get<1>(range); i++, offset++) {
                    emplace_ids(result.ids[offset], node_id, element_ids[i]);
                }
            }
        }

        // Temp. fix: When you ask for a large hyperslab in a dataset and then move
        //            to another dataset in the same file where you also ask for
        //            another large range, the next IOps take an extra few seconds.
        //            We observed that fooling HDF5 hides the issue, but we should
        //            verify this behaviour once new releases of HDF5 are available.
        const auto min_max_block = result.min_max_blocks.back();
        const auto index = result.node_index[std::get<0>(min_max_block)];
        dataset_elem_ids.select({std::get<0>(result.node_ranges[index])}, {1}).read(element_ids);
    }

    return result;
}

template <typename T>
std::pair<size_t, size_t> ReportReader<T>::Population::getIndex(
    const nonstd::optional<double>& tstart, const nonstd::optional<double>& tstop) const {
    std::pair<size_t, size_t> indexes;

    const double start = tstart.value_or(tstart_);
    const double stop = tstop.value_or(tstop_);

    if (start < 0 - EPSILON || stop < 0 - EPSILON) {
        throw SonataError("Times cannot be negative");
    }

    const auto it_start = std::find_if(times_index_.cbegin(),
                                       times_index_.cend(),
                                       [&](const std::pair<size_t, double>& v) {
                                           return start < v.second + EPSILON;
                                       });
    if (it_start == times_index_.end()) {
        throw SonataError("tstart is after the end of the range");
    }
    indexes.first = it_start->first;

    const auto it_stop =
        std::find_if(times_index_.crbegin(),
                     times_index_.crend(),
                     [&](const std::pair<size_t, double>& v) { return stop > v.second - EPSILON; });
    if (it_stop == times_index_.rend()) {
        throw SonataError("tstop is before the beginning of the range");
    }
    indexes.second = it_stop->first;

    return indexes;
}


template <typename T>
typename DataFrame<T>::DataType ReportReader<T>::Population::getNodeIdElementIdMapping(
    const nonstd::optional<Selection>& node_ids,
    const nonstd::optional<size_t>& block_gap_limit) const {
    return getNodeIdElementLayout(node_ids, block_gap_limit).ids;
}

template <typename T>
DataFrame<T> ReportReader<T>::Population::get(
    const nonstd::optional<Selection>& node_ids,
    const nonstd::optional<double>& tstart,
    const nonstd::optional<double>& tstop,
    const nonstd::optional<size_t>& tstride,
    const nonstd::optional<size_t>& block_gap_limit) const {
    size_t index_start = 0;
    size_t index_stop = 0;
    std::tie(index_start, index_stop) = getIndex(tstart, tstop);
    const size_t stride = tstride.value_or(1);
    if (stride == 0) {
        throw SonataError("tstride should be > 0");
    }
    if (index_start > index_stop) {
        throw SonataError("tstart should be <= to tstop");
    }

    // Retrieve the GID-ElementID layout, alongside the {min,max} blocks
    auto node_id_element_layout = getNodeIdElementLayout(node_ids, block_gap_limit);
    const auto& node_ranges = node_id_element_layout.node_ranges;
    const auto& node_offsets = node_id_element_layout.node_offsets;
    const auto& node_index = node_id_element_layout.node_index;
    const auto& min_max_blocks = node_id_element_layout.min_max_blocks;

    if (node_id_element_layout.ids.empty()) {  // At the end no data available (wrong node_ids?)
        return DataFrame<T>{{}, {}, {}};
    }

    // Fill times
    DataFrame<T> data_frame;
    for (size_t i = index_start; i <= index_stop; i += stride) {
        data_frame.times.emplace_back(times_index_[i].second);
    }

    // Fill ids
    data_frame.ids.swap(node_id_element_layout.ids);

    // Fill .data member
    const size_t n_time_entries = ((index_stop - index_start) / stride) + 1;
    const size_t element_ids_count = data_frame.ids.size();
    data_frame.data.resize(n_time_entries * element_ids_count);

    auto dataset = pop_group_.getDataSet("data");
    auto dataset_type = dataset.getDataType();
    if (dataset_type.getClass() != HighFive::DataTypeClass::Float || dataset_type.getSize() != 4) {
        throw SonataError(
            fmt::format("DataType of dataset 'data' should be Float32 ('{}' was found)",
                        dataset_type.string()));
    }

    std::vector<float> buffer;
    auto data_start = data_frame.data.begin();
    for (size_t timer_index = index_start; timer_index <= index_stop; timer_index += stride) {
        // Access the data in blocks to reduce the file system overhead
        for (const auto& min_max_block : min_max_blocks) {
            const auto first_index = node_index[std::get<0>(min_max_block)];
            const auto last_index = node_index[std::get<1>(min_max_block) - 1];
            const auto min = std::get<0>(node_ranges[first_index]);
            const auto max = std::get<1>(node_ranges[last_index]);

            dataset.select({timer_index, min}, {1, max - min}).read(buffer);

            // Copy the values for each of the GIDs assigned into this block
            const auto buffer_start = buffer.begin();
            for (size_t i = std::get<0>(min_max_block); i < std::get<1>(min_max_block); ++i) {
                const auto index = node_index[i];
                const auto range = Selection::Range{std::get<0>(node_ranges[index]) - min,
                                                    std::get<1>(node_ranges[index]) - min};
                const auto elements_per_gid = (std::get<1>(range) - std::get<0>(range));
                const auto offset = node_offsets[index];

                // Soma report
                if (elements_per_gid == 1) {
                    data_start[offset] = buffer_start[std::get<0>(range)];
                } else {  // Elements report
                    std::copy(std::next(buffer_start, std::get<0>(range)),
                              std::next(buffer_start, std::get<1>(range)),
                              std::next(data_start, offset));
                }
            }
        }

        std::advance(data_start, element_ids_count);
    }

    return data_frame;
}

template class ReportReader<NodeID>;
template class ReportReader<CompartmentID>;

}  // namespace sonata
}  // namespace bbp
