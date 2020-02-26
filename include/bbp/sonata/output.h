#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <bbp/sonata/population.h>

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

}  // namespace sonata
}  // namespace bbp
