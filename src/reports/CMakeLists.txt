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
        "data/node.cpp"
        "data/node.hpp"
        "data/soma_node.cpp"
        "data/soma_node.hpp"
        "data/sonata_data.cpp"
        "data/sonata_data.hpp"
        "io/io_writer.hpp"
        "io/hdf5_writer.cpp"
        "io/hdf5_writer.hpp"
        "utils/logger.cpp"
        "utils/logger.hpp")

set(reportinglib_HEADERS
        "${SONATA_INCLUDE_DIR}/bbp/reports/records.h")

set(client_SOURCES
        "client/main.cpp")

include_directories(${SONATA_INCLUDE_DIR})
include_directories(${SONATA_SRC_DIR})
include_directories(SYSTEM ${spdlog_include_directory})

add_library(reportinglib_obj OBJECT ${reportinglib_SOURCES} ${reportinglib_HEADERS})
set_property(TARGET reportinglib_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

# Shared library
add_library(sonata_report SHARED $<TARGET_OBJECTS:reportinglib_obj>)
target_link_libraries(sonata_report
    PRIVATE ${HDF5_LIBS}
    PRIVATE ${MPI_CXX_LIBRARIES})

# Coreneuron simulator Client
add_executable(client ${client_SOURCES})
target_link_libraries(client
        PRIVATE sonata_report
        PRIVATE ${MPI_CXX_LIBRARIES})

set_target_properties(sonata_report
        PROPERTIES
        OUTPUT_NAME "sonatareport"
        )
target_compile_definitions(sonata_report
        PUBLIC SONATA_DLL
        PRIVATE SONATA_DLL_EXPORTS
        )

install(TARGETS sonata_report
    EXPORT sonata-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib )
