#include <bbp/sonata/output.h>

namespace HighFive {
using namespace bbp::sonata;

H5::EnumType<SpikeReader::Population::Sorting> create_enum_sorting() {
    return H5::EnumType<SpikeReader::Population::Sorting>(
        {{"none", SpikeReader::Population::Sorting::none},
         {"by_id", SpikeReader::Population::Sorting::by_id},
         {"by_time", SpikeReader::Population::Sorting::by_time}});
}

template <>
DataType create_datatype<SpikeReader::Population::Sorting>() {
    return create_enum_sorting();
}
}  // namespace HighFive

namespace bbp {
namespace sonata {
SpikeReader::SpikeReader(const std::string& filename)
    : filename_(filename) {}

std::vector<std::string> SpikeReader::getPopulationsNames() const {
    H5::File file(filename_, H5::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikeReader::getPopulation(const std::string& populationName) const -> const Population& {
    if (populations.find(populationName) == populations.end()) {
        populations.emplace(populationName, Population{filename_, populationName});
    }

    return populations.at(populationName);
}

auto SpikeReader::operator[](const std::string& populationName) const -> const Population& {
    return getPopulation(populationName);
}

SpikeReader::Population::Spikes SpikeReader::Population::get(const Selection& node_ids,
                                                             double tstart,
                                                             double tend) const {
    auto ret = spikes;
    if (tstart != -1 && tend != -1) {
        filterTimestamp(ret, tstart, tend);
    }
    if (!node_ids.empty()) {
        filterNode(ret, node_ids);
    }
    return ret;
}

SpikeReader::Population::Sorting SpikeReader::Population::getSorting() const {
    return sorting;
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
                   std::back_inserter(spikes),
                   [](Spike::first_type&& node_id, Spike::second_type&& timestamp) {
                       return std::make_pair(std::move(node_id), std::move(timestamp));
                   });

    if (pop.hasAttribute("sorting")) {
        pop.getAttribute("sorting").read(sorting);
    }
}
void SpikeReader::Population::filterNode(Spikes& spikes, const Selection& node_ids) const {
    if (sorting == Sorting::by_id) {
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

void SpikeReader::Population::filterTimestamp(Spikes& spikes, double tstart, double tend) const {
    if (sorting == Sorting::by_time) {
        filterTimestampSorted(spikes, tstart, tend);
    }
    filterTimestampUnsorted(spikes, tstart, tend);
}

void SpikeReader::Population::filterTimestampUnsorted(Spikes& spikes,
                                                      double tstart,
                                                      double tend) const {
    auto new_end =
        std::remove_if(spikes.begin(), spikes.end(), [&tstart, &tend](const Spike& spike) {
            return spike.second < tstart || spike.second >= tend;
        });
    spikes.erase(new_end, spikes.end());
}

void SpikeReader::Population::filterTimestampSorted(Spikes& spikes,
                                                    double tstart,
                                                    double tend) const {
    auto begin = std::lower_bound(spikes.begin(),
                                  spikes.end(),
                                  std::make_pair(0UL, tstart),
                                  [](const Spike& spike1, const Spike& spike2) {
                                      return spike1.second < spike2.second;
                                  });
    auto end = std::upper_bound(spikes.begin(),
                                spikes.end(),
                                std::make_pair(0UL, tend),
                                [](const Spike& spike1, const Spike& spike2) {
                                    return spike1.second < spike2.second;
                                });
    spikes.erase(end, spikes.end());
    spikes.erase(spikes.begin(), begin);
}

ReportReader::ReportReader(const std::string& filename)
    : file(filename, H5::File::ReadOnly) {}

std::vector<std::string> ReportReader::getPopulationsNames() const {
    return file.getGroup("/report").listObjectNames();
}

auto ReportReader::getPopulation(const std::string& populationName) const -> const Population& {
    if (populations.find(populationName) == populations.end()) {
        populations.emplace(populationName, Population{file, populationName});
    }

    return populations.at(populationName);
}

auto ReportReader::operator[](const std::string& populationName) const -> const Population& {
    return getPopulation(populationName);
}

ReportReader::Population::Population(const H5::File& file, const std::string& populationName)
    : pop_group(file.getGroup(std::string("/report/") + populationName)) {
    {
        auto mapping_group = pop_group.getGroup("mapping");
        std::vector<NodeID> node_ids;
        mapping_group.getDataSet("node_ids").read(node_ids);

        std::vector<uint64_t> index_pointers;
        mapping_group.getDataSet("index_pointers").read(index_pointers);

        for (size_t i = 0; i < node_ids.size(); ++i) {
            nodes_pointers.emplace_back(node_ids[i],
                                        std::make_pair(index_pointers[i], index_pointers[i + 1]));
        }

        std::vector<double> times;
        mapping_group.getDataSet("time").read(times);
        tstart = times[0];
        tstop = times[1];
        tstep = times[2];
        mapping_group.getDataSet("time").getAttribute("units").read(time_units);

        if (mapping_group.getDataSet("node_ids").hasAttribute("sorted")) {
            mapping_group.getDataSet("node_ids").getAttribute("sorted").read(sorted);
        }
    }

    pop_group.getDataSet("data").getAttribute("units").read(data_units);
}

std::tuple<double, double, double> ReportReader::Population::getTimes() const {
    return std::tie(tstart, tstop, tstep);
}

std::string ReportReader::Population::getTimeUnits() const {
    return time_units;
}

std::string ReportReader::Population::getDataUnits() const {
    return data_units;
}

bool ReportReader::Population::getSorted() const {
    return sorted;
}

std::vector<std::vector<float>> ReportReader::Population::get(const Selection& nodes_ids,
                                                              double _tstart,
                                                              double _tstop) const {
    _tstart = _tstart == -1 ? tstart : _tstart;
    _tstop = _tstop == -1 ? tstop : _tstop;
    std::vector<std::vector<float>> ret;

    auto values = nodes_ids.flatten();
    if (values.empty()) {  // Take all nodes in this case
        std::transform(nodes_pointers.begin(),
                       nodes_pointers.end(),
                       std::back_inserter(values),
                       [](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                           return node_pointer.first;
                       });
    }

    for (const auto& value : values) {
        auto it = std::find_if(
            nodes_pointers.begin(),
            nodes_pointers.end(),
            [&value](const std::pair<NodeID, std::pair<uint64_t, uint64_t>>& node_pointer) {
                return node_pointer.first == value;
            });
        if (it == nodes_pointers.end())
            continue;

        std::vector<float> elems;
        size_t index_start = (_tstart - tstart) / tstep;
        size_t index_end = (_tstop - tstart) / tstep;
        pop_group.getDataSet("data")
            .select({index_start, it->second.first},
                    {index_end - index_start, it->second.second - it->second.first})
            .read(elems);
        ret.push_back(elems);
    }
    return ret;
}

}  // namespace sonata
}  // namespace bbp
