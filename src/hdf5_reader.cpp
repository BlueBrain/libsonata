#include <bbp/sonata/hdf5_reader.h>

#include "hdf5_reader.hpp"

namespace bbp {
namespace sonata {


Hdf5Reader::Hdf5Reader()
    : impl(std::make_shared<
           Hdf5PluginDefault<Hdf5Reader::supported_1D_types, supported_2D_types>>()) {}

Hdf5Reader::Hdf5Reader(
    std::shared_ptr<Hdf5PluginInterface<supported_1D_types, supported_2D_types>> impl)
    : impl(std::move(impl)) {}

HighFive::File Hdf5Reader::openFile(const std::string& filename) const {
    return impl->openFile(filename);
}

}  // namespace sonata
}  // namespace bbp
