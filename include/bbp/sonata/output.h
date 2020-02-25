#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <bbp/sonata/population.h>

namespace {
template <class InputIt1,
          class InputIt2,
          class OutputIt,
          class BinaryOperation,
          class BinaryPredicate>
OutputIt transform_if(InputIt1 first1,
                      InputIt1 last1,
                      InputIt2 first2,
                      OutputIt d_first,
                      BinaryOperation binary_op,
                      BinaryPredicate binary_pred) {
    while (first1 != last1) {
        if (binary_pred(*first1, *first2)) {
            *d_first++ = binary_op(*first1, *first2);
        }
        ++first1, ++first2;
    }
    return d_first;
}
}  // unnamed namespace

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
        Spikes get(Selection node_ids) const;
        Spikes get(double tstart, double tend) const;
        Sorting getSorting() const;

      private:
        Population(const std::string& filename, const std::string& populationName);

        std::vector<NodeID> node_ids;
        std::vector<double> timestamps;
        Sorting sorting = Sorting::none;

        template <class BinaryPredicate>
        Spikes get_if(BinaryPredicate filter) const {
            Spikes spikes;

            transform_if(
                node_ids.begin(),
                node_ids.end(),
                timestamps.begin(),
                std::back_inserter(spikes),
                [](NodeID node_id, double timestamp) { return std::make_pair(node_id, timestamp); },
                filter);

            return spikes;
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
