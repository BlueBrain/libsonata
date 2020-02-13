set(sonatareport_SOURCES
    "library/reports.cpp"
    "library/report.cpp"
    "library/sonatareport.cpp"
    "library/soma_report.cpp"
    "library/element_report.cpp"
    "data/node.cpp"
    "data/soma_node.cpp"
    "data/sonata_data.cpp"
    "io/hdf5_writer.cpp"
    "utils/logger.cpp"
    )

# Shared library
add_library(sonata_report SHARED ${sonatareport_SOURCES})
target_include_directories(sonata_report
    SYSTEM PRIVATE
        ${spdlog_include_directory}
    PUBLIC
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        ${MPI_INCLUDE_PATH}
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
    PRIVATE HighFive
    PRIVATE ${MPI_CXX_LIBRARIES}
)

install(TARGETS sonata_report
    EXPORT sonata-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
