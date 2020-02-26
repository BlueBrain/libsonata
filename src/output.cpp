#include <highfive/H5Easy.hpp>

#include <bbp/sonata/output.h>

namespace H5 = HighFive;

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

    std::transform(node_ids.begin(),
                   node_ids.end(),
                   timestamps.begin(),
                   std::back_inserter(spikes),
                   [](const Spike::first_type& node_id, const Spike::second_type& timestamp) {
                       return std::make_pair(node_id, timestamp);
                   });

    if (pop.hasAttribute("sorting")) {
        pop.getAttribute("sorting").read(sorting);
    }
}

}  // namespace sonata
}  // namespace bbp
