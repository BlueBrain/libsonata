configure_file(integration_test.sh.in integration_test.sh @ONLY)

add_test(NAME integration_test
        COMMAND "/bin/sh" ${CMAKE_CURRENT_BINARY_DIR}/integration_test.sh
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")