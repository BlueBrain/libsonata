#include <bbp/sonata/output.h>

H5::EnumType<bbp::sonata::SpikeReader::Population::Sorting> create_enum_sorting() {
    using namespace bbp::sonata;
    return H5::EnumType<SpikeReader::Population::Sorting>(
        {{"none", SpikeReader::Population::Sorting::none},
         {"by_id", SpikeReader::Population::Sorting::by_id},
         {"by_time", SpikeReader::Population::Sorting::by_time}});
}

HIGHFIVE_REGISTER_TYPE(bbp::sonata::SpikeReader::Population::Sorting, create_enum_sorting)

namespace bbp {
namespace sonata {
SpikeReader::SpikeReader(const std::string& filename)
    : filename_(filename) {}

std::vector<std::string> SpikeReader::getPopulationsNames() const {
    H5::File file(filename_, H5::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikeReader::getPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{filename_, populationName});
    }

    return populations_.at(populationName);
}

auto SpikeReader::operator[](const std::string& populationName) const -> const Population& {
    return getPopulation(populationName);
}

SpikeReader::Population::Spikes SpikeReader::Population::get(const Selection& node_ids,
                                                             double tstart,
                                                             double tstop) const {
    auto ret = spikes_;
    tstart = tstart == -1 ? 0 : tstart;
    tstop = tstop == -1 ? 99999 : tstop;  // FIXME
    filterTimestamp(ret, tstart, tstop);
    if (!node_ids.empty()) {
        filterNode(ret, node_ids);
    }
    return ret;
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
}
void SpikeReader::Population::filterNode(Spikes& spikes, const Selection& node_ids) const {
    if (sorting_ == Sorting::by_id) {
        filterNodeIDSorted(spikes, node_ids);
    }
    filterNodeIDUnsorted(spikes, node_ids);
}

void SpikeReader::Population::filterNodeIDUnsorted(Spikes& spikes,
                                                   const Selection& node_ids) const {
    auto values = node_ids.flatten();
    auto new_end = std::remove_if(spikes.begin(), spikes.end(), [&values](const Spike& spike) {
        return std::find(values.cbegin(), values.cend(), spike.first) == values.cend();
    });
    spikes.erase(new_end, spikes.end());
}

void SpikeReader::Population::filterNodeIDSorted(Spikes& spikes, const Selection& node_ids) const {
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
                                    std::make_pair(range.second, 0.),
                                    [](const Spike& spike1, const Spike& spike2) {
                                        return spike1.first < spike2.first;
                                    });

        std::move(begin, end, std::back_inserter(_spikes));
        spikes.erase(begin, end);  // have to erase, because otherwise it is no more sorted
    }
    spikes = std::move(_spikes);
}

void SpikeReader::Population::filterTimestamp(Spikes& spikes, double tstart, double tstop) const {
    if (sorting_ == Sorting::by_time) {
        filterTimestampSorted(spikes, tstart, tstop);
    }
    filterTimestampUnsorted(spikes, tstart, tstop);
}

void SpikeReader::Population::filterTimestampUnsorted(Spikes& spikes,
                                                      double tstart,
                                                      double tstop) const {
    auto new_end =
        std::remove_if(spikes.begin(), spikes.end(), [&tstart, &tstop](const Spike& spike) {
            return spike.second < tstart || spike.second >= tstop;
        });
    spikes.erase(new_end, spikes.end());
}

void SpikeReader::Population::filterTimestampSorted(Spikes& spikes,
                                                    double tstart,
                                                    double tstop) const {
    auto begin = std::lower_bound(spikes.begin(),
                                  spikes.end(),
                                  std::make_pair(0UL, tstart),
                                  [](const Spike& spike1, const Spike& spike2) {
                                      return spike1.second < spike2.second;
                                  });
    auto end = std::upper_bound(spikes.begin(),
                                spikes.end(),
                                std::make_pair(0UL, tstop),
                                [](const Spike& spike1, const Spike& spike2) {
                                    return spike1.second < spike2.second;
                                });
    spikes.erase(end, spikes.end());
    spikes.erase(spikes.begin(), begin);
}

template <typename T>
ReportReader<T>::ReportReader(const std::string& filename)
    : file_(filename, H5::File::ReadOnly) {}

template <typename T>
std::vector<std::string> ReportReader<T>::getPopulationsNames() const {
    return file_.getGroup("/report").listObjectNames();
}

template <typename T>
auto ReportReader<T>::getPopulation(const std::string& populationName) const -> const Population& {
    if (populations_.find(populationName) == populations_.end()) {
        populations_.emplace(populationName, Population{file_, populationName});
    }

    return populations_.at(populationName);
}

template <typename T>
auto ReportReader<T>::operator[](const std::string& populationName) const -> const Population& {
    return getPopulation(populationName);
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

        for (size_t i = 0; i < node_ids.size() - 1; ++i) {
            nodes_pointers_.emplace_back(node_ids[i],
                                         std::make_pair(index_pointers[i], index_pointers[i + 1]));
        }
        auto size = pop_group_.getDataSet("data").getDimensions();
        nodes_pointers_.emplace_back(node_ids.back(),
                                     std::make_pair(index_pointers.back(), size[1]));

        {  // Get times
            std::vector<double> times;
            mapping_group.getDataSet("time").read(times);
            tstart_ = times[0];
            tstop_ = times[1];
            tstep_ = times[2];
            mapping_group.getDataSet("time").getAttribute("units").read(time_units_);
        }

        if (mapping_group.getDataSet("node_ids").hasAttribute("sorted")) {
            mapping_group.getDataSet("node_ids").getAttribute("sorted").read(nodes_ids_sorted_);
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
std::pair<T, std::vector<float>> make_value(NodeID node_id,
                                            uint32_t compartment_id,
                                            std::vector<float> values);

template <>
std::pair<NodeID, std::vector<float>> make_value(NodeID node_id,
                                                 uint32_t /* compartment_id */,
                                                 std::vector<float> values) {
    return {node_id, values};
}

template <>
std::pair<std::pair<NodeID, uint32_t>, std::vector<float>> make_value(NodeID node_id,
                                                                      uint32_t compartment_id,
                                                                      std::vector<float> values) {
    return {{node_id, compartment_id}, values};
}

template <typename T>
DataFrame<T> ReportReader<T>::Population::get(const Selection& nodes_ids,
                                              double tstart,
                                              double tstop) const {
    tstart = tstart < tstart_ ? tstart_ : tstart;
    tstart = tstart > tstop_ ? tstart_ : tstart;
    tstop = tstop < tstart_ ? tstop_ : tstop;
    tstop = tstop > tstop_ ? tstop_ : tstop;
    size_t index_start = (tstart - tstart_) / tstep_;
    tstart = index_start * tstep_;
    size_t index_stop = (tstop - tstart_) / tstep_;
    tstop = index_stop * tstep_;

    DataFrame<T> ret;
    for (auto t = tstart; t < tstop && std::abs(t - tstop) > std::numeric_limits<double>::epsilon();
         t += tstep_) {
        ret.index.push_back(t);
    }

    // Simplify selection
    // We should remove duplicates
    // And when we can work with ranges let's sort them
    // auto nodes_ids_ = Selection::fromValues(nodes_ids.flatten().sort());
    auto nodes_ids_ = nodes_ids;

    if (nodes_ids.empty()) {  // Take all nodes in this case
        Selection::Values values;
        std::transform(nodes_pointers_.begin(),
                       nodes_pointers_.end(),
                       std::back_inserter(values),
                       [](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                           return node_pointer.first;
                       });
        nodes_ids_ = Selection::fromValues(values);
    }

    // It will be good to do it for ranges but if nodes_ids are not sorted it is not easy
    // TODO: specialized this function for sorted nodes_ids
    for (const auto& value : nodes_ids_.flatten()) {
        auto it = std::find_if(
            nodes_pointers_.begin(),
            nodes_pointers_.end(),
            [&value](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                return node_pointer.first == value;
            });
        if (it == nodes_pointers_.end())
            continue;

        // elems are by timestamp and by Nodes_id
        std::vector<std::vector<float>> elems;
        pop_group_.getDataSet("data")
            .select({index_start, it->second.first},
                    {index_stop - index_start, it->second.second - it->second.first})
            .read(elems);

        std::vector<uint32_t> element_ids;
        pop_group_.getGroup("mapping")
            .getDataSet("element_ids")
            .select({it->second.first}, {it->second.second - it->second.first})
            .read(element_ids);
        for (size_t i = 0; i < element_ids.size(); ++i) {
            std::vector<float> elems_by_node;
            for (auto& elem : elems) {
                elems_by_node.push_back(elem[i]);
            }
            ret.data.insert(make_value<T>(value, element_ids[i], std::move(elems_by_node)));
        }
    }
    return ret;
}

template class ReportReader<NodeID>;
template class ReportReader<std::pair<NodeID, uint32_t>>;
}  // namespace sonata
}  // namespace bbp
