file(GLOB all_TESTS test_*.cpp)
set(TEST_SOURCES
        tests.cpp
        ${all_TESTS}
        )

add_executable(unit-tests ${TEST_SOURCES})
target_link_libraries(unit-tests
    PRIVATE Catch2::Catch2
    PRIVATE reportinglib
)
catch_discover_tests(unit-tests)