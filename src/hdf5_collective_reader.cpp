#include "population.hpp"
#include <cstdint>
#include <stdexcept>

#include "read_bulk.hpp"

namespace bbp {
namespace sonata {

class Hdf5PluginCollectiveData
{
  public:
    Hdf5PluginCollectiveData() = default;
    Hdf5PluginCollectiveData(MPI_Comm comm, bool collective_metadata, bool collective_transfer)
        : comm(comm_dup(comm))
        , collective_metadata(collective_metadata)
        , collective_transfer(collective_transfer) {}

    ~Hdf5PluginCollectiveData() {
        MPI_Comm_free(&comm);
    }

    HighFive::FileAccessProps fapl() const {
        HighFive::FileAccessProps fapl;
        fapl.add(HighFive::MPIOFileAccess{MPI_COMM_WORLD, MPI_INFO_NULL});
        fapl.add(HighFive::MPIOCollectiveMetadata{collective_metadata});

        return fapl;
    }

    HighFive::DataTransferProps dxpl() const {
        HighFive::DataTransferProps dxpl;
        dxpl.add(HighFive::UseCollectiveIO{collective_transfer});

        return dxpl;
    }

  private:
    MPI_Comm comm_dup(MPI_Comm old_comm) {
        MPI_Comm new_comm;
        MPI_Comm_dup(old_comm, &new_comm);
        return new_comm;
    }

    MPI_Comm comm = MPI_COMM_SELF;
    bool collective_metadata = false;
    bool collective_transfer = false;
};


namespace detail {
template <class Range>
HighFive::HyperSlab make_hyperslab(const std::vector<Range>& ranges) {
    HighFive::HyperSlab slab;
    for (const auto& range : ranges) {
        size_t i_begin = std::get<0>(range);
        size_t i_end = std::get<1>(range);
        slab |= HighFive::RegularHyperSlab({i_begin}, {i_end - i_begin});
    }

    return slab;
}

template <class Range>
HighFive::HyperSlab make_hyperslab(const std::vector<Range>& xranges,
                                   const std::array<size_t, 2>& yslice) {
    HighFive::HyperSlab slab;

    size_t j_begin = std::get<0>(yslice);
    size_t j_end = std::get<1>(yslice);
    for (const auto& range : xranges) {
        size_t i_begin = std::get<0>(range);
        size_t i_end = std::get<1>(range);
        slab |= HighFive::RegularHyperSlab({i_begin, j_begin}, {i_end - i_begin, j_end - j_begin});
    }

    return slab;
}

template <class T, class Range>
std::vector<T> readCanonicalSelection(const HighFive::DataSet& dset,
                                      const std::vector<Range>& ranges,
                                      const HighFive::DataTransferProps& dxpl) {
    return dset.select(make_hyperslab(ranges)).template read<std::vector<T>>(dxpl);
}

template <class Range>
std::vector<std::array<uint64_t, 2>> readCanonicalRow(const HighFive::DataSet& dset,
                                                      const std::vector<Range>& ranges,
                                                      const HighFive::DataTransferProps& dxpl) {
    auto dataspace = HighFive::DataSpace({bulk_read::detail::flatSize(ranges), 2});
    return dset.select(make_hyperslab(ranges, {0, 2}), dataspace)
        .template read<std::vector<std::array<uint64_t, 2>>>(dxpl);
}

template <class T, class Range>
std::vector<T> readSelection(const HighFive::DataSet& dset,
                             const std::vector<Range>& ranges,
                             const HighFive::DataTransferProps& dxpl) {
    if (bulk_read::detail::isCanonical(ranges)) {
        return readCanonicalSelection<T>(dset, ranges, dxpl);
    }

    // The fully general case:
    //
    // 1. Create a canonical selection and read into `linear_result`.
    // 2. Copy values from the canonical `linear_results` to their final
    //    destination.
    auto canonicalRanges = bulk_read::sortAndMerge(ranges, 0);
    auto linear_result = readCanonicalSelection<T>(dset, canonicalRanges, dxpl);

    const auto ids = bulk_read::detail::flattenRanges(ranges);

    std::vector<std::size_t> ids_index(ids.size());
    std::iota(ids_index.begin(), ids_index.end(), std::size_t(0));
    std::stable_sort(ids_index.begin(), ids_index.end(), [&ids](size_t i0, size_t i1) {
        return ids[i0] < ids[i1];
    });

    std::vector<T> result(ids.size());
    size_t linear_index = 0;
    result[ids_index[0]] = linear_result[0];
    for (size_t i = 1; i < ids.size(); ++i) {
        if (ids[ids_index[i - 1]] != ids[ids_index[i]]) {
            linear_index += 1;
        }

        result[ids_index[i]] = linear_result[linear_index];
    }

    return result;
}
}  // namespace detail

template <class T>
class Hdf5PluginReadCollective: virtual public Hdf5PluginCollectiveData,
                                virtual public Hdf5PluginReadInterface<T>
{
  public:
    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const Selection& selection) const override {
        return detail::readSelection<T>(dset, selection.ranges(), this->dxpl());
    }
};

template <class T>
class Hdf5PluginCollective;

template <class... Types>
class Hdf5PluginCollective<std::tuple<Types...>>
    : virtual public Hdf5PluginInterface<std::tuple<Types...>>,
      virtual public Hdf5PluginReadCollective<Types>...
{
  public:
    Hdf5PluginCollective(MPI_Comm comm, bool collective_metadata, bool collective_transfer)
        : Hdf5PluginCollectiveData(comm, collective_metadata, collective_transfer) {}

    HighFive::File openFile(const std::string& path) const override {
        return HighFive::File(path,
                              HighFive::File::ReadOnly,
                              static_cast<const Hdf5PluginCollectiveData&>(*this).fapl());
    }

    std::vector<std::array<uint64_t, 2>> readRows(const HighFive::DataSet& dset,
                                                  const Selection& selection) const override {
        return detail::readCanonicalRow(dset, selection.ranges(), this->dxpl());
    }

    std::vector<std::array<uint64_t, 2>> readRows(
        const HighFive::DataSet& dset,
        const std::vector<std::array<uint64_t, 2>>& ranges) const override {
        return detail::readCanonicalRow(dset, ranges, this->dxpl());
    }
};

Hdf5Reader make_collective_reader(MPI_Comm comm,
                                  bool collective_metadata,
                                  bool collective_transfer) {
    return Hdf5Reader(std::make_shared<Hdf5PluginCollective<Hdf5Reader::supported_types>>(
        comm, collective_metadata, collective_transfer));
}


}  // namespace sonata
}  // namespace bbp
