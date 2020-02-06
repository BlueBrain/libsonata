set(sonatareport_SOURCES
        "library/reports.cpp"
        "library/report.cpp"
        "library/report.hpp"
        "library/sonatareport.cpp"
        "library/sonatareport.hpp"
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

set(client_SOURCES
        "client/main.cpp")

include_directories(SYSTEM ${spdlog_include_directory})

# Shared library
add_library(sonata_report SHARED ${sonatareport_SOURCES})
target_include_directories(sonata_report
        PUBLIC
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${SONATA_SRC_DIR}>
        ${MPI_INCLUDE_PATH}
        ${HDF5_INCLUDE_DIRS}
        $<INSTALL_INTERFACE:include>
        )
target_compile_options(sonata_report
        PRIVATE ${SONATA_COMPILE_OPTIONS}
        )
if (ENABLE_COVERAGE)
    target_compile_options(sonata_report
            PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage
            )
    target_link_libraries(sonata_report
            PRIVATE gcov
            )
endif()
target_compile_definitions(sonata_report
        PUBLIC SONATA_DLL
        PRIVATE SONATA_DLL_EXPORTS
        )
set_target_properties(sonata_report
        PROPERTIES
        OUTPUT_NAME "sonatareport"
        )
target_link_libraries(sonata_report
        PRIVATE ${HDF5_LIBS}
        PRIVATE ${MPI_CXX_LIBRARIES}
        )

# Coreneuron simulator Client
add_executable(client ${client_SOURCES})
target_link_libraries(client
        PRIVATE sonata_report
        PRIVATE ${MPI_CXX_LIBRARIES}
        )

install(TARGETS sonata_report
        EXPORT sonata-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
