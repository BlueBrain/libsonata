set(SONATA_PYTHON_BUILD ${CMAKE_CURRENT_BINARY_DIR}/libsonata)

if (EXTLIB_FROM_SUBMODULES)
    add_subdirectory(pybind11 EXCLUDE_FROM_ALL)
else()
    find_package(pybind11)
endif()

pybind11_add_module(sonata_python SYSTEM bindings.cpp)

set_target_properties(sonata_python PROPERTIES
    OUTPUT_NAME "_libsonata"
)

target_link_libraries(sonata_python
    PRIVATE sonata_static
    PRIVATE HighFive
    PRIVATE fmt::fmt-header-only
    PRIVATE pybind11::module
)
