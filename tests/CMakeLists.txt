set(TESTS_SRC
  main.cpp
  test_edges.cpp
)

add_executable(test_all ${TESTS_SRC})
target_link_libraries(test_all sonata)

add_test(NAME test_all COMMAND test_all)
