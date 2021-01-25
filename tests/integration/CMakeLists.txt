configure_file(integration_test.sh.in integration_test.sh @ONLY)

# Coreneuron simulator Client
add_executable(reports_integration_test integration_test.cpp)
target_include_directories(reports_integration_test
    PRIVATE
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${SONATA_SRC_DIR}>
    SYSTEM
        ${PROJECT_SOURCE_DIR}/extlib/spdlog/include
)
target_compile_options(reports_integration_test
    PRIVATE ${SONATA_COMPILE_OPTIONS}
)
target_link_libraries(reports_integration_test
    PUBLIC sonata_report
    PRIVATE spdlog::spdlog_header_only
    PRIVATE ${MPI_CXX_LIBRARIES}
)

add_test(NAME reports_integration_test
    COMMAND "/bin/sh" ${CMAKE_CURRENT_BINARY_DIR}/integration_test.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)