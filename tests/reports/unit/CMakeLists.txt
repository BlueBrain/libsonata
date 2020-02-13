set(TEST_SOURCES
    tests.cpp
    test_node.cpp
    test_report.cpp
    test_sonatadata.cpp
    test_sonatareport.cpp
    )

add_executable(reports_unit_tests ${TEST_SOURCES})
target_include_directories(reports_unit_tests
    PRIVATE
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${SONATA_SRC_DIR}>
    SYSTEM
        ${spdlog_include_directory}
)
target_link_libraries(reports_unit_tests
    PRIVATE
        sonata_report
        Catch2::Catch2
        HighFive
        ${MPI_CXX_LIBRARIES}
)

catch_discover_tests(reports_unit_tests)
