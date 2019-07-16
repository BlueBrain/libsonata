set(reportinglib_SOURCES
        "library/cell.cpp"
        "library/cell.hpp"
        "library/records.cpp"
        "library/report.cpp"
        "library/report.hpp"
        "library/reportinglib.cpp"
        "library/reportinglib.hpp"
        "library/soma_report.cpp"
        "library/soma_report.hpp"
        "library/compartment_report.cpp"
        "library/compartment_report.hpp"
        "library/spike_report.cpp"
        "library/spike_report.hpp"
        "library/report_format.cpp"
        "library/report_format.hpp"
        "library/sonata_format.cpp"
        "library/sonata_format.hpp"
        "library/io_writer.cpp"
        "library/io_writer.hpp"
        "library/ascii_writer.cpp"
        "library/ascii_writer.hpp"
        "library/hdf5_writer.cpp"
        "library/hdf5_writer.hpp")

set(reportinglib_HEADERS
        "${SONATA_INCLUDE_DIR}/bbp/reports/records.hpp")

set(client_SOURCES
        "client/main.cpp")

#include_directories(SYSTEM ${spdlog_include_directory})

add_library(reportinglib_obj OBJECT ${reportinglib_SOURCES} ${reportinglib_HEADERS})
set_property(TARGET reportinglib_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

# Shared library
add_library(reportinglib SHARED $<TARGET_OBJECTS:reportinglib_obj>)
target_link_libraries(reportinglib ${HDF5_LIBS} ${MPI_CXX_LIBRARIES})

# Coreneuron simulator Client
add_executable(client ${client_SOURCES})
target_link_libraries(client PRIVATE reportinglib)