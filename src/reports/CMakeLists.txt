set(reportinglib_SOURCES
        "library/records.cpp"
        "library/report.cpp"
        "library/report.hpp"
        "library/reportinglib.cpp"
        "library/reportinglib.hpp"
        "library/soma_report.cpp"
        "library/soma_report.hpp"
        "library/element_report.cpp"
        "library/element_report.hpp"
        "library/spike_report.cpp"
        "library/spike_report.hpp"
        "data/node.cpp"
        "data/node.hpp"
        "data/sonata_data.cpp"
        "data/sonata_data.hpp"
        "io/io_writer.hpp"
        "io/hdf5_writer.cpp"
        "io/hdf5_writer.hpp")

set(reportinglib_HEADERS
        "${SONATA_INCLUDE_DIR}/bbp/reports/records.h")

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