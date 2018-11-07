#pragma once

#include <mutex>

// Every access to hdf5 must be serialized if HDF5 does not take care of it
// which needs a thread-safe built of the library.
// https://support.hdfgroup.org/HDF5/faq/threadsafe.html
#define HDF5_LOCK_GUARD std::lock_guard<std::mutex> _hdf5_lock(hdf5Mutex());


namespace bbp {
namespace sonata {

std::mutex& hdf5Mutex();

}} // namespace bbp::sonata
