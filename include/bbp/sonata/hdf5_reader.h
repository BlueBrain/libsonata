#pragma once

#include <tuple>
#include <vector>

#include <bbp/sonata/selection.h>
#include <highfive/H5File.hpp>

namespace bbp {
namespace sonata {

/// Interface for implementing `readSelection<T>(dset, selection)`.
template <class T>
class Hdf5PluginRead1DInterface
{
  public:
    virtual ~Hdf5PluginRead1DInterface() = default;

    /// Read the selected subset of the one-dimensional array.
    ///
    /// The selection is canonical, i.e. sorted and non-overlapping. The dataset
    /// is obtained from a `HighFive::File` opened via `this->openFile`.
    virtual std::vector<T> readSelection(const HighFive::DataSet& dset,
                                         const Selection& selection) const = 0;
};

template <class T>
class Hdf5PluginRead2DInterface
{
  public:
    virtual ~Hdf5PluginRead2DInterface() = default;

    /// Read the Cartesian product of the two selections.
    ///
    /// Both selections are canonical, i.e. sorted and non-overlapping. The dataset
    /// is obtained from a `HighFive::File` opened via `this->openFile`.
    virtual std::vector<std::array<uint64_t, 2>> readSelection(const HighFive::DataSet& dset,
                                                               const Selection& xsel,
                                                               const Selection& ysel) const = 0;
};

template <class T, class U>
class Hdf5PluginInterface;

/// Interface of Plugins for reading HDF5 datasets.
///
/// All method must be called in an MPI-collective manner. Each method is free
/// to break any MPI collective requirements.
template <class... Ts, class... Us>
class Hdf5PluginInterface<std::tuple<Ts...>, std::tuple<Us...>>
    : virtual public Hdf5PluginRead1DInterface<Ts>...,
      virtual public Hdf5PluginRead2DInterface<Us>...
{
  public:
    /// Open the HDF5 file.
    ///
    /// This allows setting File Access Properties.
    virtual HighFive::File openFile(const std::string& path) const = 0;
};

/// Abstraction for reading HDF5 datasets.
///
/// The Hdf5Reader provides an interface for reading canonical selections from
/// datasets. Selections are canonical if they are sorted and don't overlap.
/// This allows implementing different optimization strategies, such as
/// minimizing bytes read, aggregating nearby reads or using MPI collective I/O.
///
/// The design uses virtual inheritance, which enables users to inject their own
/// reader if needed. This class is the interface used within libsonata. It
/// simply delegates to a "plugin", that satisfies the interface
/// `Hdf5PluginInterface`.
///
/// To enable MPI collective I/O, `libsonata` must call all methods in an
/// MPI-collective manner. This implies that the number of times any function in
/// `libsonata` calls any of the `Hdf5Reader` methods must not depend on the
/// arguments to the function.
///
/// Examples:
///
///    void wrong(Selection selection) {
///      // Wrong because some MPI ranks might return without
///      // calling `readSelection`.
///      if(selection.empty()) {
///        return;
///      }
///      hdf5_reader.readSelection(dset, selection);
///    }
///
///    void also_wrong(Selection selection) {
///      // Wrong because `hdf5_reader` is called `selection.ranges().size()`
///      // number of times. Which could be different on each MPI rank.
///      for(auto range : selection.ranges()) {
///        hdf5_reader.readSelection(dset, Selection(std::vector{range}));
///      }
///    }
///
///    void subtle(Selection selection, bool flag) {
///      // If the flag can differ between MPI ranks, this is wrong because
///      // `readSelection` is called with different `dset`s. If the `flag` must
///      // be the same on all MPI ranks, this is correct. If this happens in
///      // the libsonata API, then passing the same `flag` on all MPI ranks becomes
///      // a requirement for the users, when using a collective reader. Example:
///      //     pop.get_attribute(attr_name, selection)
///      if(flag) {
///        hdf5_reader.readSelection(dset1, selection);
///      } else {
///        hdf5_reader.readSelection(dset2, selection);
///      }
///    }
///
///    void correct(Selection selection) {
///      // Correct because no matter which branch is taken
///      // `hdf5_reader.readSelection` is called exactly once.
///      if(selection.size % 2 == 0) {
///        hdf5_reader.readSelection(dset, selection);
///      } else {
///        hdf5_reader.readSelection(dset, {});
///      }
///    }
///
class SONATA_API Hdf5Reader
{
  public:
    // The issue here is that on a mac `size_t` is different from
    // `{,u}int{8,16,32,64}_t` but not on the other two OSes.
    using supported_1D_types = std::tuple<uint8_t,
                                          uint16_t,
                                          uint32_t,
                                          uint64_t,
                                          int8_t,
                                          int16_t,
                                          int32_t,
                                          int64_t,
                                          float,
                                          double,
#ifdef __APPLE__
                                          size_t,
#endif
                                          std::string>;

    using supported_2D_types = std::tuple<std::array<uint64_t, 2>>;

    /// Create a valid Hdf5Reader with the default plugin.
    Hdf5Reader();

    /// Create an Hdf5Reader with a user supplied plugin.
    Hdf5Reader(std::shared_ptr<Hdf5PluginInterface<supported_1D_types, supported_2D_types>> impl);

    /// Read the selected subset of the one-dimensional array.
    ///
    /// Both selections are canonical, i.e. sorted and non-overlapping. The dataset
    /// is obtained from a `HighFive::File` opened via `this->openFile`.
    template <class T>
    std::vector<T> readSelection(const HighFive::DataSet& dset, const Selection& selection) const {
        return static_cast<const Hdf5PluginRead1DInterface<T>&>(*impl).readSelection(dset,
                                                                                     selection);
    }

    /// Open the HDF5.
    ///
    /// The dataset passed to `readSelection` must be obtained from a file open
    /// via this method.
    HighFive::File openFile(const std::string& filename) const;

    /// Read the Cartesian product of the two selections.
    ///
    /// Both selections are canonical, i.e. sorted and non-overlapping. The dataset
    /// is obtained from a `HighFive::File` opened via `this->openFile`.
    template <class T>
    std::vector<T> readSelection(const HighFive::DataSet& dset,
                                 const Selection& xsel,
                                 const Selection& ysel) const {
        return static_cast<const Hdf5PluginRead2DInterface<T>&>(*impl).readSelection(dset,
                                                                                     xsel,
                                                                                     ysel);
    }

  private:
    std::shared_ptr<Hdf5PluginInterface<supported_1D_types, supported_2D_types>> impl;
};

}  // namespace sonata
}  // namespace bbp
