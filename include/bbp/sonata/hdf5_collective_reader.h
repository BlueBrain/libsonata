#pragma once

#include <mpi.h>

#include <bbp/sonata/hdf5_reader.h>

namespace bbp {
namespace sonata {

Hdf5Reader make_collective_reader(MPI_Comm comm,
                                  bool collective_metadata,
                                  bool collective_transfer);

}
}  // namespace bbp
