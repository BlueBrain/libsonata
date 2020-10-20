#include <bbp/sonata/report_reader.h>
#include <fmt/format.h>

constexpr double EPSILON = 1e-6;

H5::EnumType<bbp::sonata::SpikeReader::Population::Sorting> create_enum_sorting() {
    using bbp::sonata::SpikeReader;
    return H5::EnumType<SpikeReader::Population::Sorting>(
        {{"none", SpikeReader::Population::Sorting::none},
         {"by_id", SpikeReader::Population::Sorting::by_id},
         {"by_time", SpikeReader::Population::Sorting::by_time}});
}

HIGHFIVE_REGISTER_TYPE(bbp::sonata::SpikeReader::Population::Sorting, create_enum_sorting)

namespace {

using bbp::sonata::ElementID;
using bbp::sonata::NodeID;
using bbp::sonata::Selection;
using bbp::sonata::Spike;
using bbp::sonata::Spikes;

void filterNodeIDUnsorted(Spikes& spikes, const Selection& node_ids) {
    const auto values = node_ids.flatten();
    const auto new_end =
        std::remove_if(spikes.begin(), spikes.end(), [&values](const Spike& spike) {
            return std::find(values.cbegin(), values.cend(), spike.first) == values.cend();
        });
    spikes.erase(new_end, spikes.end());
}

void filterNodeIDSorted(Spikes& spikes, const Selection& node_ids) {
    Spikes _spikes;
    for (const auto& range : node_ids.ranges()) {
        const auto begin = std::lower_bound(spikes.begin(),
                                            spikes.end(),
                                            std::make_pair(range.first, 0.),
                                            [](const Spike& spike1, const Spike& spike2) {
                                                return spike1.first < spike2.first;
                                            });
        const auto end = std::upper_bound(spikes.begin(),
                                          spikes.end(),
                                          std::make_pair(range.second - 1, 0.),
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

template <typename T>
T make_key(NodeID node_id, ElementID element_id);

template <>
NodeID make_key(NodeID node_id, ElementID /* element_id */) {
    return node_id;
}

template <>
std::pair<NodeID, ElementID> make_key(NodeID node_id, ElementID element_id) {
    return {node_id, element_id};
}

}  // anonymous namespace

namespace bbp {
namespace sonata {

SpikeReader::SpikeReader(const std::string& filename)
    : filename_(filename) {}

std::vector<std::string> SpikeReader::getPopulationNames() const {
    H5::File file(filename_, H5::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikeReader::openPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{filename_, populationName});
    }

    return populations_.at(populationName);
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

    auto spikes = spikes_;
    filterTimestamp(spikes, start, stop);

    if (node_ids) {
        filterNode(spikes, node_ids.value());
    }

    return spikes;
}

SpikeReader::Population::Sorting SpikeReader::Population::getSorting() const {
    return sorting_;
}

SpikeReader::Population::Population(const std::string& filename,
                                    const std::string& populationName) {
    H5::File file(filename, H5::File::ReadOnly);
    const auto pop_path = std::string("/spikes/") + populationName;
    const auto pop = file.getGroup(pop_path);

    std::vector<Spike::first_type> node_ids;
    pop.getDataSet("node_ids").read(node_ids);

    std::vector<Spike::second_type> timestamps;
    pop.getDataSet("timestamps").read(timestamps);

    if (node_ids.size() != timestamps.size()) {
        throw SonataError(
            "In spikes file, 'node_ids' and 'timestamps' does not have the same size.");
    }

    std::transform(std::make_move_iterator(node_ids.begin()),
                   std::make_move_iterator(node_ids.end()),
                   std::make_move_iterator(timestamps.begin()),
                   std::back_inserter(spikes_),
                   [](Spike::first_type&& node_id, Spike::second_type&& timestamp) {
                       return std::make_pair(std::move(node_id), std::move(timestamp));
                   });

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
    : file_(filename, H5::File::ReadOnly) {}

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
ReportReader<T>::Population::Population(const H5::File& file, const std::string& populationName)
    : pop_group_(file.getGroup(std::string("/report/") + populationName)) {
    {
        const auto mapping_group = pop_group_.getGroup("mapping");
        mapping_group.getDataSet("node_ids").read(nodes_ids_);

        std::vector<uint64_t> index_pointers;
        mapping_group.getDataSet("index_pointers").read(index_pointers);

        for (size_t i = 0; i < nodes_ids_.size(); ++i) {
            nodes_pointers_.emplace(nodes_ids_[i],
                                    std::make_pair(index_pointers[i], index_pointers[i + 1]));
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

        if (mapping_group.getDataSet("node_ids").hasAttribute("sorted")) {
            uint8_t sorted;
            mapping_group.getDataSet("node_ids").getAttribute("sorted").read(sorted);
            nodes_ids_sorted_ = sorted != 0;
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
    return nodes_ids_sorted_;
}

template <typename T>
std::vector<NodeID> ReportReader<T>::Population::getNodeIds() const {
    return nodes_ids_;
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
DataFrame<T> ReportReader<T>::Population::get(const nonstd::optional<Selection>& selection,
                                              const nonstd::optional<double>& tstart,
                                              const nonstd::optional<double>& tstop,
                                              const nonstd::optional<size_t>& tstride) const {
    DataFrame<T> data_frame;
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

    for (size_t i = index_start; i <= index_stop; i += stride) {
        data_frame.times.push_back(times_index_[i].second);
    }

    // Simplify selection
    // We should remove duplicates
    // And when we can work with ranges let's sort them
    // auto nodes_ids_ = Selection::fromValues(node_ids.flatten().sort());
    Selection::Values node_ids;

    if (!selection) {  // Take all nodes in this case
        node_ids.reserve(nodes_pointers_.size());
        std::transform(nodes_pointers_.begin(),
                       nodes_pointers_.end(),
                       std::back_inserter(node_ids),
                       [](const std::pair<NodeID, Range>& node_pointer) {
                           return node_pointer.first;
                       });
    } else if (selection->empty()) {
        return DataFrame<T>{{}, {}, {}};
    } else {
        node_ids = selection->flatten();
    }

    Ranges positions;
    // min and max offsets of the node_ids requested are calculated
    // to reduce the amount of IO that is brought to memory
    uint64_t min = std::numeric_limits<uint64_t>::max();
    uint64_t max = std::numeric_limits<uint64_t>::min();
    auto dataset_elem_ids = pop_group_.getGroup("mapping").getDataSet("element_ids");
    for (const auto& node_id : node_ids) {
        const auto it = nodes_pointers_.find(node_id);
        if (it == nodes_pointers_.end()) {
            continue;
        }
        min = std::min(it->second.first, min);
        max = std::max(it->second.second, max);
        positions.emplace_back(it->second.first, it->second.second);

        std::vector<ElementID> element_ids(it->second.second - it->second.first);
        dataset_elem_ids.select({it->second.first}, {it->second.second - it->second.first})
            .read(element_ids.data());
        for (const auto& elem : element_ids) {
            data_frame.ids.push_back(make_key<T>(node_id, elem));
        }
    }
    if (data_frame.ids.empty()) {  // At the end no data available (wrong node_ids?)
        return DataFrame<T>{{}, {}, {}};
    }

    // Fill .data member
    size_t n_time_entries = ((index_stop - index_start) / stride) + 1;
    size_t n_ids = data_frame.ids.size();
    data_frame.data.resize(n_time_entries * n_ids);

    auto dataset = pop_group_.getDataSet("data");
    auto dataset_type = dataset.getDataType();
    if (dataset_type.getClass() != HighFive::DataTypeClass::Float || dataset_type.getSize() != 4) {
        throw SonataError(
            fmt::format("DataType of dataset 'data' should be Float32 ('{}' was found)",
                        dataset_type.string()));
    }
    std::vector<float> buffer(max - min);
    for (size_t timer_index = index_start; timer_index <= index_stop; timer_index += stride) {
        // Note: The code assumes that the file is chunked by rows and not by columns
        // (i.e., if the chunking changes in the future, the reading method must also be adapted)
        dataset.select({timer_index, min}, {1, max - min}).read(buffer.data());

        off_t offset = 0;
        off_t data_offset = (timer_index - index_start) / stride;
        auto data_ptr = &data_frame.data[data_offset * n_ids];
        for (const auto& position : positions) {
            uint64_t elements_per_gid = position.second - position.first;
            uint64_t gid_start = position.first - min;

            // Soma report
            if (elements_per_gid == 1) {
                data_ptr[offset] = buffer[gid_start];
            } else {  // Elements report
                uint64_t gid_end = position.second - min;
                std::copy(&buffer[gid_start], &buffer[gid_end], &data_ptr[offset]);
            }
            offset += elements_per_gid;
        }
    }
    return data_frame;
}

template class ReportReader<NodeID>;
template class ReportReader<std::pair<NodeID, ElementID>>;

}  // namespace sonata
}  // namespace bbp
