#include <bbp/sonata/report_reader.h>

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

using bbp::sonata::NodeID;
using bbp::sonata::Selection;
using bbp::sonata::Spike;
using bbp::sonata::Spikes;

void filterNodeIDUnsorted(Spikes& spikes, const Selection& node_ids) {
    auto values = node_ids.flatten();
    auto new_end = std::remove_if(spikes.begin(), spikes.end(), [&values](const Spike& spike) {
        return std::find(values.cbegin(), values.cend(), spike.first) == values.cend();
    });
    spikes.erase(new_end, spikes.end());
}

void filterNodeIDSorted(Spikes& spikes, const Selection& node_ids) {
    Spikes _spikes;
    for (const auto& range : node_ids.ranges()) {
        auto begin = std::lower_bound(spikes.begin(),
                                      spikes.end(),
                                      std::make_pair(range.first, 0.),
                                      [](const Spike& spike1, const Spike& spike2) {
                                          return spike1.first < spike2.first;
                                      });
        auto end = std::upper_bound(spikes.begin(),
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
    auto end = std::upper_bound(spikes.begin(),
                                spikes.end(),
                                std::make_pair(0UL, tstop + EPSILON),
                                [](const Spike& spike1, const Spike& spike2) {
                                    return spike1.second < spike2.second;
                                });
    spikes.erase(end, spikes.end());
    auto begin = std::lower_bound(spikes.begin(),
                                  spikes.end(),
                                  std::make_pair(0UL, tstart - EPSILON),
                                  [](const Spike& spike1, const Spike& spike2) {
                                      return spike1.second < spike2.second;
                                  });
    spikes.erase(spikes.begin(), begin);
}

template <typename T>
T make_key(NodeID node_id, uint32_t element_id);

template <>
NodeID make_key(NodeID node_id, uint32_t /* element_id */) {
    return node_id;
}

template <>
std::pair<NodeID, uint32_t> make_key(NodeID node_id, uint32_t element_id) {
    return {node_id, element_id};
}

}  // anonymous namespace

namespace bbp {
namespace sonata {

SpikeReader::SpikeReader(const std::string& filename)
    : filename_(filename) {}

std::vector<std::string> SpikeReader::getPopulationsNames() const {
    H5::File file(filename_, H5::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikeReader::openPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{filename_, populationName});
    }

    return populations_.at(populationName);
}

Spikes SpikeReader::Population::get(const Selection& node_ids, double tstart, double tstop) const {
    tstart = tstart < 0 ? tstart_ : tstart;
    tstop = tstop < 0 ? tstop_ : tstop;
    if (tstart > tstop_ + EPSILON || tstop < tstart_ - EPSILON || tstop < tstart) {
        return Spikes{};
    }

    auto spikes = spikes_;
    filterTimestamp(spikes, tstart, tstop);

    if (!node_ids.empty()) {
        filterNode(spikes, node_ids);
    }

    return spikes;
}

SpikeReader::Population::Sorting SpikeReader::Population::getSorting() const {
    return sorting_;
}

SpikeReader::Population::Population(const std::string& filename,
                                    const std::string& populationName) {
    H5::File file(filename, H5::File::ReadOnly);
    auto pop_path = std::string("/spikes/") + populationName;
    auto pop = file.getGroup(pop_path);

    std::vector<Spike::first_type> node_ids;
    pop.getDataSet("node_ids").read(node_ids);

    std::vector<Spike::second_type> timestamps;
    pop.getDataSet("timestamps").read(timestamps);

    if (node_ids.size() != timestamps.size()) {
        throw std::runtime_error(
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
        tstart_ = timestamps.front();
        tstop_ = timestamps.back();
    } else {
        tstart_ = *min_element(timestamps.cbegin(), timestamps.cend());
        tstop_ = *max_element(timestamps.cbegin(), timestamps.cend());
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
std::vector<std::string> ReportReader<T>::getPopulationsNames() const {
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
        auto mapping_group = pop_group_.getGroup("mapping");
        std::vector<NodeID> node_ids;
        mapping_group.getDataSet("node_ids").read(node_ids);

        std::vector<uint64_t> index_pointers;
        mapping_group.getDataSet("index_pointers").read(index_pointers);

        for (size_t i = 0; i < node_ids.size(); ++i) {
            nodes_pointers_.emplace_back(node_ids[i],
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
            for (double t = tstart_; t < tstop_ - EPSILON; t += tstep_) {
                times_index_.push_back({i++, t});
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
std::pair<size_t, size_t> ReportReader<T>::Population::getIndex(double tstart, double tstop) const {
    std::pair<size_t, size_t> indexes;
    if (tstart < 0 - EPSILON) {  // Default value
        indexes.first = times_index_.front().first;
    } else if (tstart > times_index_.back().second + EPSILON) {  // tstart is after end of range
        indexes.first = 1;
        indexes.second = 0;
        return indexes;
    } else {
        for (const auto& time_index : times_index_) {
            if (tstart < time_index.second + EPSILON) {
                indexes.first = time_index.first;
                break;
            }
        }
    }
    if (tstop < 0 - EPSILON) {  // Default value
        indexes.second = times_index_.back().first;
    } else if (tstop < times_index_.front().second - EPSILON) {  // tstop is before start of range
        indexes.first = 1;
        indexes.second = 0;
        return indexes;
    } else {
        for (auto it = times_index_.crbegin(); it != times_index_.crend(); ++it) {
            const auto& time_index = *it;
            if (tstop > time_index.second - EPSILON) {
                indexes.second = time_index.first;
                break;
            }
        }
    }

    if (indexes.first > indexes.second) {
        indexes.second = indexes.first;
    }

    return indexes;
}

template <typename T>
DataFrame<T> ReportReader<T>::Population::get(const Selection& selection,
                                              double tstart,
                                              double tstop) const {
    DataFrame<T> data_frame;

    size_t index_start = 0;
    size_t index_stop = 0;

    std::tie(index_start, index_stop) = getIndex(tstart, tstop);
    if (index_start > index_stop) {
        return data_frame;
    }

    for (size_t i = index_start; i <= index_stop; ++i) {
        data_frame.times.push_back(times_index_[i].second);
    }

    // Simplify selection
    // We should remove duplicates
    // And when we can work with ranges let's sort them
    // auto nodes_ids_ = Selection::fromValues(nodes_ids.flatten().sort());
    Selection::Values node_ids;

    if (selection.empty()) {  // Take all nodes in this case
        std::transform(nodes_pointers_.begin(),
                       nodes_pointers_.end(),
                       std::back_inserter(node_ids),
                       [](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                           return node_pointer.first;
                       });
    } else {
        node_ids = selection.flatten();
    }

    data_frame.data.resize(index_stop - index_start + 1);
    // FIXME: It will be good to do it for ranges but if nodes_ids are not sorted it is not easy
    // TODO: specialized this function for sorted nodes_ids?
    for (const auto& node_id : node_ids) {
        auto it = std::find_if(
            nodes_pointers_.begin(),
            nodes_pointers_.end(),
            [&node_id](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                return node_pointer.first == node_id;
            });
        if (it == nodes_pointers_.end()) {
            continue;
        }

        // elems are by timestamp and by Nodes_id
        std::vector<std::vector<float>> data;
        pop_group_.getDataSet("data")
            .select({index_start, it->second.first},
                    {index_stop - index_start + 1, it->second.second - it->second.first})
            .read(data);
        int timer_index = 0;
        for (const std::vector<float>& datum : data) {
            for (float d : datum) {
                data_frame.data[timer_index].push_back(d);
            }
            ++timer_index;
        }

        std::vector<uint32_t> element_ids;
        pop_group_.getGroup("mapping")
            .getDataSet("element_ids")
            .select({it->second.first}, {it->second.second - it->second.first})
            .read(element_ids);
        for (size_t i = 0; i < element_ids.size(); ++i) {
            data_frame.ids.push_back(make_key<T>(node_id, element_ids[i]));
        }
    }
    return data_frame;
}

template class ReportReader<NodeID>;
template class ReportReader<std::pair<NodeID, uint32_t>>;

}  // namespace sonata
}  // namespace bbp
