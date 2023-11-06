#pragma once

#include <mpi.h>

#include <highfive/H5File.hpp>

namespace bbp {
namespace sonata {

// -- generic -----------------------------------------------------------------
template <class Hdf5Props>
class IoOptsImpl
{
  public:
    using hdf5_props_t = Hdf5Props;
    virtual void apply(hdf5_props_t& fapl) const = 0;
};

template <class Impl, class Default>
class IoAspectOpts
{
  public:
    using hdf5_props_t = typename Impl::hdf5_props_t;

    IoAspectOpts()
        : impl(std::make_shared<Default>()) {}
    IoAspectOpts(std::shared_ptr<Impl> impl)
        : impl(std::move(impl)) {}

    void apply(hdf5_props_t& fapl) const {
        if (impl) {
            return impl->apply(fapl);
        }
        throw SonataError("Invalid IoOpts, `impl == nullptr`.");
    }

  private:
    std::shared_ptr<Impl> impl;
};

// -- FileAccessOpts ----------------------------------------------------------
using FileAccessOptsImpl = IoOptsImpl<HighFive::FileAccessProps>;

class NoFileAccessOptsImpl: public FileAccessOptsImpl
{
  public:
    void apply(HighFive::FileAccessProps&) const override {}
};

using FileAccessOpts = IoAspectOpts<FileAccessOptsImpl, NoFileAccessOptsImpl>;


// -- TransferOpts ------------------------------------------------------------
using DataTransferOptsImpl = IoOptsImpl<HighFive::DataTransferProps>;

class NoDataTransferOptsImpl: public DataTransferOptsImpl
{
  public:
    void apply(HighFive::DataTransferProps&) const override {}
};

using DataTransferOpts = IoAspectOpts<DataTransferOptsImpl, NoDataTransferOptsImpl>;

// -- IoOpts -------------------------------------------------------------------
class IoOpts: public FileAccessOpts, public DataTransferOpts
{
  public:
    IoOpts() = default;
    IoOpts(FileAccessOpts file_access_opts, DataTransferOpts data_transfer_opts)
        : FileAccessOpts(std::move(file_access_opts))
        , DataTransferOpts(std::move(data_transfer_opts)) {}
    using DataTransferOpts::apply;
    using FileAccessOpts::apply;
};

}  // namespace sonata
}  // namespace bbp
