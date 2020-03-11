set(TESTS_SRC
  main.cpp
  test_edges.cpp
  test_nodes.cpp
  test_report_reader.cpp
  test_selection.cpp
)

add_executable(unittests ${TESTS_SRC})
target_link_libraries(unittests
    PRIVATE
    sonata_shared
    HighFive
    Catch2::Catch2
)
add_test(NAME unittests COMMAND unittests WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
