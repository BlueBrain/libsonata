#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <highfive/H5Easy.hpp>

#include <bbp/sonata/population.h>

namespace H5 = HighFive;

namespace bbp {
namespace sonata {
/**
  const SpikeReader file(filename);
  auto pops = file.getPopulationNames();
  for (const auto& data: file[pops[0]].get(Selection{12UL, 34UL, 58UL})) {
      NodeID node_id;
      double timestamp;
      std::tie(node_id, timestamp) = data;
      std::cout << "[" << timestamp << "] " << node_id << std::endl;
  }
*/
class SONATA_API SpikeReader
{
  public:
    class Population
    {
      public:
        enum class Sorting : char {
            none = 0,
            by_id = 1,
            by_time = 2,
        };

        using Spike = std::pair<NodeID, double>;
        using Spikes = std::vector<Spike>;
        Spikes get() const;
        Spikes get(const Selection& node_ids) const;
        Spikes get(double tstart, double tend) const;
        Spikes get(const Selection& node_ids, double tstart, double tend) const;
        Sorting getSorting() const;

      private:
        Population(const std::string& filename, const std::string& populationName);

        Spikes spikes;
        Sorting sorting = Sorting::none;

        // Helpers to filter by node_ids
        Spikes filterNode(const Spikes& spikes, const Selection& node_ids) const;
        Spikes filterNodeIDUnsorted(const Spikes& spikes, const Selection& node_ids) const;
        Spikes filterNodeIDSorted(const Spikes& spikes, const Selection& node_ids) const;

        // Helpers to filter by timestamps
        Spikes filterTimestamp(const Spikes& spikes, double tstart, double tend) const;
        Spikes filterTimestampUnsorted(const Spikes& spikes, double tstart, double tend) const;
        Spikes filterTimestampSorted(const Spikes& spikes, double tstart, double tend) const;

        friend SpikeReader;
    };

    explicit SpikeReader(const std::string& filename);

    std::vector<std::string> getPopulationsNames() const;

    const Population& getPopulation(const std::string& populationName) const;

    const Population& operator[](const std::string& populationName) const;

  private:
    std::string filename_;

    // Lazy loaded population
    mutable std::map<std::string, Population> populations;
};

class SONATA_API ReportReader
{
  public:
    class Population
    {
      public:
        std::tuple<double, double, double> getTimes() const {
            return std::tie(tstart, tstop, tstep);
        }

        std::string getTimeUnits() const {
            return time_units;
        }

        std::string getDataUnits() const {
            return data_units;
        }

        bool getSorted() const {
            return sorted;
        }

        // Return a vector of datas
        // Each index is a corresponding node_id from Selection given as argument
        std::vector<std::vector<float>> get(const Selection& nodes_ids,
                                            double _tstart = -1,
                                            double _tstop = -1) const;

      private:
        Population(const H5::File& file, const std::string& populationName);

        std::vector<std::pair<NodeID, std::pair<uint64_t, uint64_t>>> nodes_pointers;
        H5::Group pop_group;
        double tstart, tstop, tstep;
        std::string time_units;
        std::string data_units;
        bool sorted = false;

        friend ReportReader;
    };

    explicit ReportReader(const std::string& filename);

    std::vector<std::string> getPopulationsNames() const;

    const Population& getPopulation(const std::string& populationName) const;

    const Population& operator[](const std::string& populationName) const;

  private:
    H5::File file;

    // Lazy loaded population
    mutable std::map<std::string, Population> populations;
};

}  // namespace sonata
}  // namespace bbp
