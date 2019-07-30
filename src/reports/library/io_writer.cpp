#include <iostream>

#include "io_writer.hpp"
#include "ascii_writer.hpp"
#include "hdf5_writer.hpp"

IoWriter::IoWriter(const std::string& report_name): m_report_name(report_name) {}

std::unique_ptr<IoWriter> IoWriter::create_io_writer(IoWriter::Kind kind, const std::string &report_name) {

    switch(kind) {
        case IoWriter::Kind::HDF5:
            return std::make_unique<HDF5Writer>(report_name);
        case IoWriter::Kind::ASCII:
            return std::make_unique<AsciiWriter>(report_name);
        case IoWriter::Kind::MPIIO:
            std::cout << "MPI I/O writer not implemented yet!" << std::endl;
            break;
        default:
            std::cout << "Report format type '" << static_cast<int>(kind) << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}
