#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <highfive/H5File.hpp>

#include <bbp/sonata/population.h>

namespace H5 = HighFive;

namespace bbp {
namespace sonata {

// KeyType will be NodeID for somas report and pair<NodeID, uint32_t> for elements report
template <typename KeyType>
struct SONATA_API DataFrame {
    using DataType = std::map<KeyType, std::vector<float>>;
    std::vector<double> index;
    DataType data;
};

using Spike = std::pair<NodeID, double>;
using Spikes = std::vector<Spike>;

/**
  const SpikeReader file(filename);
  auto pops = file.getPopulationNames();
  for (const auto& data: file[pops.openPopulation(0).get(Selection{12UL, 34UL, 58UL})) {
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

        Spikes get(const Selection& node_ids = Selection({}),
                   double tstart = -1,
                   double tstop = -1) const;
        Sorting getSorting() const;

      private:
        Population(const std::string& filename, const std::string& populationName);

        Spikes spikes_;
        Sorting sorting_ = Sorting::none;
        // Use for clamping of user values
        double tstart_, tstop_;

        void filterNode(Spikes& spikes, const Selection& node_ids) const;
        void filterTimestamp(Spikes& spikes, double tstart, double tstop) const;

        friend SpikeReader;
    };

    explicit SpikeReader(const std::string& filename);

    std::vector<std::string> getPopulationsNames() const;

    const Population& openPopulation(const std::string& populationName) const;

  private:
    std::string filename_;

    // Lazy loaded population
    mutable std::map<std::string, Population> populations_;
};

template <typename KeyType>
class SONATA_API ReportReader
{
  public:
    class Population
    {
      public:
        std::tuple<double, double, double> getTimes() const;
        std::string getTimeUnits() const;
        std::string getDataUnits() const;
        bool getSorted() const;

        DataFrame<KeyType> get(const Selection& nodes_ids = Selection({}),
                               double _tstart = -1,
                               double _tstop = -1) const;

      private:
        Population(const H5::File& file, const std::string& populationName);
        std::pair<size_t, size_t> getIndex(double tstart, double tstop) const;

        std::vector<std::pair<NodeID, std::pair<uint64_t, uint64_t>>> nodes_pointers_;
        H5::Group pop_group_;
        double tstart_, tstop_, tstep_;
        std::vector<std::pair<size_t, double>> times_index_;
        std::string time_units_;
        std::string data_units_;
        bool nodes_ids_sorted_ = false;

        friend ReportReader;
    };

    explicit ReportReader(const std::string& filename);

    std::vector<std::string> getPopulationsNames() const;

    const Population& openPopulation(const std::string& populationName) const;

  private:
    H5::File file_;

    // Lazy loaded population
    mutable std::map<std::string, Population> populations_;
};

using SomaReportReader = ReportReader<NodeID>;
using ElementReportReader = ReportReader<std::pair<NodeID, uint32_t>>;

}  // namespace sonata
}  // namespace bbp
