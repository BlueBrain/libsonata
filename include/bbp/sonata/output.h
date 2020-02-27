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

        Spikes filterNode(const Spikes& spikes, const Selection& node_ids) const {
            if (sorting == Sorting::by_id) {
                return filterNodeIDSorted(spikes, node_ids);
            }
            return filterNodeIDUnsorted(spikes, node_ids);
        }

        Spikes filterNodeIDUnsorted(const Spikes& spikes, const Selection& node_ids) const {
            auto values = node_ids.flatten();
            Spikes _spikes;
            std::copy_if(spikes.begin(),
                         spikes.end(),
                         std::back_inserter(_spikes),
                         [&values](const Spike& spike) {
                             return std::find(values.cbegin(), values.cend(), spike.first) !=
                                    values.cend();
                         });
            return _spikes;
        }

        Spikes filterNodeIDSorted(const Spikes& spikes, const Selection& node_ids) const {
            auto values = node_ids.flatten();
            Spikes _spikes;
            for (const auto& value : values) {
                auto range = std::equal_range(spikes.begin(),
                                              spikes.end(),
                                              std::make_pair(value, 0.),
                                              [](const Spike& spike1, const Spike& spike2) {
                                                  return spike1.first < spike2.first;
                                              });
                std::copy(range.first, range.second, std::back_inserter(_spikes));
            }
            return _spikes;
        }

        Spikes filterTimestamp(const Spikes& spikes, double tstart, double tend) const {
            if (sorting == Sorting::by_time) {
                return filterTimestampSorted(spikes, tstart, tend);
            }
            return filterTimestampUnsorted(spikes, tstart, tend);
        }

        Spikes filterTimestampUnsorted(const Spikes& spikes, double tstart, double tend) const {
            Spikes _spikes;
            std::copy_if(spikes.begin(),
                         spikes.end(),
                         std::back_inserter(_spikes),
                         [&tstart, &tend](const Spike& spike) {
                             return spike.second > tstart && spike.second < tend;
                         });
            return _spikes;
        }

        Spikes filterTimestampSorted(const Spikes& spikes, double tstart, double tend) const {
            Spikes _spikes;
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
            std::copy(begin, end, std::back_inserter(_spikes));
            return _spikes;
        }

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
                                            double _tstop = -1) const {
            _tstart = _tstart == -1 ? tstart : _tstart;
            _tstop = _tstop == -1 ? tstop : _tstop;
            std::vector<std::vector<float>> ret;

            auto values = nodes_ids.flatten();
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

      private:
        Population(const H5::File& file, const std::string& populationName);

        std::vector<std::pair<NodeID, std::pair<uint64_t, uint64_t>>> nodes_pointers;
        H5::Group pop_group;
        double tstart = 0, tstop, tstep = 1. /* Remove me with valid datafile*/;
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
