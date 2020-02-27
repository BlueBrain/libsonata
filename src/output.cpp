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

SpikeReader::Population::Spikes SpikeReader::Population::get() const {
    return spikes;
}

SpikeReader::Population::Spikes SpikeReader::Population::get(const Selection& node_ids) const {
    return filterNode(spikes, node_ids);
}

SpikeReader::Population::Spikes SpikeReader::Population::get(double tstart, double tend) const {
    return filterTimestamp(spikes, tstart, tend);
}

SpikeReader::Population::Spikes SpikeReader::Population::get(const Selection& node_ids,
                                                             double tstart,
                                                             double tend) const {
    return filterNode(filterTimestamp(spikes, tstart, tend), node_ids);
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

        // std::vector<double> times;
        // mapping_group.getDataSet("time").read(times);
        // tstart = times[0];
        // tstop  = times[1];
        // tstep  = times[2];
        // mapping_group.getDataSet("time").getAttribute("units").read(time_units);

        if (mapping_group.hasAttribute("sorted")) {
            mapping_group.getAttribute("sorted").read(sorted);
        }
    }

    // pop_group.getDataSet("data").getAttribute("units").read(data_units);
}

}  // namespace sonata
}  // namespace bbp
