file(GLOB all_TESTS test_*.cpp)
set(TEST_SOURCES
        tests.cpp
        ${all_TESTS}
        )

include_directories(SYSTEM ${spdlog_include_directory})

add_executable(unit-tests ${TEST_SOURCES})
target_include_directories(unit-tests
    PRIVATE
    $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${SONATA_SRC_DIR}>
)
target_link_libraries(unit-tests
    PRIVATE
    sonata_report
    Catch2::Catch2
    ${MPI_CXX_LIBRARIES}
)

catch_discover_tests(unit-tests)
