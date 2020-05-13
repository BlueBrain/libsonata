if(REPORTS_ONLY)
    add_subdirectory(reports)
else()
    set(TESTS_SRC
      main.cpp
      test_edges.cpp
      test_nodes.cpp
      test_selection.cpp
    )
    add_executable(unittests ${TESTS_SRC})
    target_link_libraries(unittests
        PRIVATE
        sonata_shared
        Catch2::Catch2
    )

    add_test(NAME unittests COMMAND unittests WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

    add_subdirectory(reports)
endif()
