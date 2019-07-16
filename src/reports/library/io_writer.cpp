#include <stdlib.h>
#include <new>
#include <iostream>
#include <stdlib.h>

#include "io_writer.hpp"
#include "ascii_writer.hpp"
#include "hdf5_writer.hpp"

IoWriter::IoWriter(const std::string& report_name): m_reportName(report_name) {}
IoWriter::~IoWriter() {}

std::unique_ptr<IoWriter> IoWriter::create_IoWriter(const KindWriter& kind, const std::string& report_name) {

    switch(kind) {
        case HDF5:
            return std::make_unique<HDF5Writer>(report_name);
        case ASCII:
            return std::make_unique<AsciiWriter>(report_name);
        case MPIIO:
            std::cout << "MPI I/O writer not implemented yet!" << std::endl;
            break;
        default:
            std::cout << "Report format type '" << kind << "' does not exist!" << std::endl;
            break;
    }
    return nullptr;
}
