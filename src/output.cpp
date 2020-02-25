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
    std::vector<Spike> spikes;

    std::transform(node_ids.begin(),
                   node_ids.end(),
                   timestamps.begin(),
                   std::back_inserter(spikes),
                   [](NodeID node_id, double timestamp) {
                       return std::make_pair(node_id, timestamp);
                   });

    return spikes;
}

SpikeReader::Population::Spikes SpikeReader::Population::get(Selection node_ids) const {
    auto values = node_ids.flatten();
    return get_if([&values](NodeID node_id_, double) {
        return std::find(values.begin(), values.end(), node_id_) != values.end();
    });
}

SpikeReader::Population::Spikes SpikeReader::Population::get(double tstart, double tend) const {
    return get_if([&tstart, &tend](NodeID, double timestamp) {
        return timestamp > tstart && timestamp < tend;
    });
}

SpikeReader::Population::Sorting SpikeReader::Population::getSorting() const {
    return sorting;
}

SpikeReader::Population::Population(const std::string& filename,
                                    const std::string& populationName) {
    H5::File file(filename, H5::File::ReadOnly);

    node_ids = H5Easy::load<decltype(node_ids)>(file,
                                                std::string("/spikes/") + populationName +
                                                    "/node_ids");
    timestamps = H5Easy::load<decltype(timestamps)>(file,
                                                    std::string("/spikes/") + populationName +
                                                        "/timestamps");
    if (file.getGroup(std::string("/spikes/") + populationName).hasAttribute("sorting")) {
        auto g = file.getGroup(std::string("/spikes/") + populationName);
        g.getAttribute("sorting").read(sorting);
    }

    if (node_ids.size() != timestamps.size()) {
        throw std::runtime_error(
            "In spikes file 'node_ids' and 'timestamps' does not have the same size.");
    }
}

}  // namespace sonata
}  // namespace bbp
