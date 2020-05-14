find_package(HDF5 REQUIRED)
add_library(HighFive INTERFACE)
target_include_directories(HighFive INTERFACE
    HighFive/include/
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFive INTERFACE
    ${HDF5_C_LIBRARIES})

add_subdirectory(fmt)

if (SONATA_TESTS)
    add_subdirectory(Catch2)
endif()
