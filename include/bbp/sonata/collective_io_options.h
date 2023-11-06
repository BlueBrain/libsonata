#pragma once

#if SONATA_HAS_MPI == 1
#include <bbp/sonata/io_options.h>

namespace bbp {
namespace sonata {

class MPIFileAccessOptsImpl: public FileAccessOptsImpl
{
  public:
    MPIFileAccessOptsImpl() = default;
    MPIFileAccessOptsImpl(MPI_Comm comm, bool collective)
        : comm(comm)
        , collective_metadata(collective) {}
    ~MPIFileAccessOptsImpl() {
        MPI_Comm_free(&comm);
    }

    void apply(HighFive::FileAccessProps& fapl) const override {
        fapl.add(HighFive::MPIOFileAccess{MPI_COMM_WORLD, MPI_INFO_NULL});
        fapl.add(HighFive::MPIOCollectiveMetadata{collective_metadata});
    }

  private:
    MPI_Comm comm_dup(MPI_Comm old_comm) {
        MPI_Comm new_comm;
        MPI_Comm_dup(old_comm, &new_comm);
        return new_comm;
    }

    MPI_Comm comm = MPI_COMM_NULL;
    bool collective_metadata = false;
};

class MPIDataTransferOptsImpl: public DataTransferOptsImpl
{
  public:
    MPIDataTransferOptsImpl() = default;
    MPIDataTransferOptsImpl(bool collective)
        : collective_transfer(collective) {}

    void apply(HighFive::DataTransferProps& dxpl) const override {
        dxpl.add(HighFive::UseCollectiveIO{collective_transfer});
    }

  private:
    bool collective_transfer = false;
};


}  // namespace sonata
}  // namespace bbp

#endif
