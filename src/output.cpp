#include <highfive/H5Easy.hpp>

#include <bbp/sonata/output.h>

namespace H5 = HighFive;

namespace bbp {
namespace sonata {
SpikesReader::SpikesReader(const std::string& filename)
    : filename_(filename) {}

std::vector<std::string> SpikesReader::getPopulationsNames() const {
    H5::File file(filename_, H5::File::ReadOnly);
    return file.getGroup("/spikes").listObjectNames();
}

auto SpikesReader::getPopulation(const std::string& populationName) const -> const Population& {
    if (populations.find(populationName) == populations.end()) {
        populations.emplace(populationName, Population{filename_, populationName});
    }

    return populations.at(populationName);
}

auto SpikesReader::operator[](const std::string& populationName) const -> const Population& {
    return getPopulation(populationName);
}

std::vector<std::pair<uint64_t, double>> SpikesReader::Population::get() const {
    std::vector<std::pair<uint64_t, double>> vec;

    transform(node_ids.begin(),
              node_ids.end(),
              timestamps.begin(),
              std::back_inserter(vec),
              [](uint64_t node_id, double timestamp) {
                  return std::make_pair(node_id, timestamp);
              });

    return vec;
}

std::vector<std::pair<uint64_t, double>> SpikesReader::Population::get(Selection node_ids) const {
    auto values = node_ids.flatten();
    return get_if([&values](uint64_t node_id_, double) {
        return std::find(values.begin(), values.end(), node_id_) != values.end();
    });
}

std::vector<std::pair<uint64_t, double>> SpikesReader::Population::get(double tstart,
                                                                       double tend) const {
    return get_if([&tstart, &tend](uint64_t, double timestamp) {
        return timestamp > tstart && timestamp < tend;
    });
}

SpikesReader::Population::Population(const std::string& filename,
                                     const std::string& populationName) {
    H5::File file(filename, H5::File::ReadOnly);

    node_ids = H5Easy::load<decltype(node_ids)>(file,
                                                std::string("/spikes/") + populationName +
                                                    "/node_ids");
    timestamps = H5Easy::load<decltype(timestamps)>(file,
                                                    std::string("/spikes/") + populationName +
                                                        "/timestamps");

    if (node_ids.size() != timestamps.size()) {
        throw std::runtime_error(
            "In spikes file 'node_ids' and 'timestamps' does not have the same size.");
    }
}

}  // namespace sonata
}  // namespace bbp
