#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <highfive/H5File.hpp>

#include <bbp/sonata/optional.hpp>
#include <bbp/sonata/population.h>

namespace bbp {
namespace sonata {

// KeyType will be NodeID for somas report and CompartmentID for elements report
template <typename KeyType>
struct SONATA_API DataFrame {
    using DataType = std::vector<KeyType>;
    std::vector<double> times;
    DataType ids;
    // data[times][ids], flattened. n_cols is ids.size()
    std::vector<float> data;
};

using Spike = std::pair<NodeID, double>;
using Spikes = std::vector<Spike>;
struct SpikeTimes {
    std::vector<NodeID> node_ids;
    std::vector<double> timestamps;
};

/// Used to read spike files
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

        /**
         * Return (tstart, tstop) of the population
         */
        std::tuple<double, double> getTimes() const;

        /**
         * Return spikes with all those node_ids between 'tstart' and 'tstop'
         */
        Spikes get(const nonstd::optional<Selection>& node_ids = nonstd::nullopt,
                   const nonstd::optional<double>& tstart = nonstd::nullopt,
                   const nonstd::optional<double>& tstop = nonstd::nullopt) const;

        /**
         * Return the raw node_ids and timestamps vectors
         */
        const SpikeTimes& getRawArrays() const;

        /**
         * Return the node_ids and timestamps vectors with all node_ids between 'tstart' and 'tstop'
         */
        SpikeTimes getArrays(const nonstd::optional<Selection>& node_ids = nonstd::nullopt,
                             const nonstd::optional<double>& tstart = nonstd::nullopt,
                             const nonstd::optional<double>& tstop = nonstd::nullopt) const;

        /**
         * Return the way data are sorted ('none', 'by_id', 'by_time')
         */
        Sorting getSorting() const;

        /**
         * Return the unit of time
         */
        std::string getTimeUnits() const;

      private:
        Population(const std::string& filename, const std::string& populationName);

        SpikeTimes spike_times_;
        Sorting sorting_ = Sorting::none;
        // Use for clamping of user values
        double tstart_, tstop_;
        std::string time_units_;

        void filterNode(Spikes& spikes, const Selection& node_ids) const;
        void filterTimestamp(Spikes& spikes, double tstart, double tstop) const;
        /**
         * Create the spikes from the vectors of node_ids and timestamps
         */
        Spikes createSpikes() const;

        friend SpikeReader;
    };

    explicit SpikeReader(std::string filename);

    /**
     * Return a list of all population names.
     */
    std::vector<std::string> getPopulationNames() const;

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
        /**
         * Return (tstart, tstop, tstep) of the population
         */
        std::tuple<double, double, double> getTimes() const;

        /**
         * Return the unit of time
         */
        std::string getTimeUnits() const;

        /**
         * Return the unit of data.
         */
        std::string getDataUnits() const;

        /**
         * Return true if the data is sorted.
         */
        bool getSorted() const;

        /**
         * Return all the node ids.
         */
        std::vector<NodeID> getNodeIds() const;

        /**
         * Return the ElementIds for the passed Node.
         * The return type will depend on the report reader:
         * - For Soma report reader, the return value will be the Node ID to which the report
         *   value belongs to.
         * - For Element/full compartment readers, the return value will be an array with 2
         *   elements, the first element is the Node ID and the second element is the
         *   compartment ID of the given Node.
         *
         * \param node_ids limit the report to the given selection. If nullptr, all nodes in the
         * report are used
         * \param block_gap_limit gap limit between each IO block while fetching data from storage
         */
        typename DataFrame<KeyType>::DataType getNodeIdElementIdMapping(
            const nonstd::optional<Selection>& node_ids = nonstd::nullopt,
            const nonstd::optional<size_t>& block_gap_limit = nonstd::nullopt) const;

        /**
         * \param node_ids limit the report to the given selection.
         * \param tstart return voltages occurring on or after tstart. tstart=nonstd::nullopt
         * indicates no limit. \param tstop return voltages occurring on or before tstop.
         * tstop=nonstd::nullopt indicates no limit. \param tstride indicates every how many
         * timesteps we read data. tstride=nonstd::nullopt indicates that all timesteps are read.
         * \param block_gap_limit gap limit between each IO block while fetching data from storage.
         */
        DataFrame<KeyType> get(
            const nonstd::optional<Selection>& node_ids = nonstd::nullopt,
            const nonstd::optional<double>& tstart = nonstd::nullopt,
            const nonstd::optional<double>& tstop = nonstd::nullopt,
            const nonstd::optional<size_t>& tstride = nonstd::nullopt,
            const nonstd::optional<size_t>& block_gap_limit = nonstd::nullopt) const;

      private:
        struct NodeIdElementLayout {
            typename DataFrame<KeyType>::DataType ids;
            Selection::Ranges node_ranges;
            std::vector<uint64_t> node_offsets;
            std::vector<uint64_t> node_index;
            Selection::Ranges min_max_blocks;
        };

        Population(const HighFive::File& file, const std::string& populationName);
        std::pair<size_t, size_t> getIndex(const nonstd::optional<double>& tstart,
                                           const nonstd::optional<double>& tstop) const;
        /**
         * Return the element IDs for the given selection, alongside the filtered node pointers
         * and the range of positions where they fit in the file. This latter two are necessary
         * for performance to understand how and where to retrieve the data from storage.
         *
         * \param node_ids limit the report to the given selection. If nullptr, all nodes in the
         * report are used
         * \param block_gap_limit gap limit between each IO block while fetching data from storage
         */
        NodeIdElementLayout getNodeIdElementLayout(
            const nonstd::optional<Selection>& node_ids = nonstd::nullopt,
            const nonstd::optional<size_t>& block_gap_limit = nonstd::nullopt) const;

        HighFive::Group pop_group_;
        std::vector<NodeID> node_ids_;
        std::vector<Selection::Range> node_ranges_;
        std::vector<uint64_t> node_offsets_;
        std::vector<uint64_t> node_index_;
        double tstart_, tstop_, tstep_;
        std::vector<std::pair<size_t, double>> times_index_;
        std::string time_units_;
        std::string data_units_;
        bool is_node_ids_sorted_;

        friend ReportReader;
    };

    explicit ReportReader(const std::string& filename);

    /**
     * Return a list of all population names.
     */
    std::vector<std::string> getPopulationNames() const;

    const Population& openPopulation(const std::string& populationName) const;

  private:
    HighFive::File file_;

    // Lazy loaded population
    mutable std::map<std::string, Population> populations_;
};

using SomaReportReader = ReportReader<NodeID>;
using ElementReportReader = ReportReader<CompartmentID>;

}  // namespace sonata
}  // namespace bbp
