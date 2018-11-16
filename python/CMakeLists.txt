set(SONATA_PYTHON_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sonata)

if (EXTLIB_FROM_SUBMODULES)
    add_subdirectory(pybind11 EXCLUDE_FROM_ALL)
else()
    find_package(pybind11)
endif()

add_library(sonata_python MODULE bindings.cpp)
set_target_properties(sonata_python PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_sonata"
    LIBRARY_OUTPUT_DIRECTORY ${SONATA_PYTHON_BUILD}
)

target_link_libraries(sonata_python
    PRIVATE sonata_static
    PRIVATE HighFive
    PRIVATE fmt::fmt-header-only
    PRIVATE pybind11::module
)

configure_file(
    sonata/__init__.py.in
    ${SONATA_PYTHON_BUILD}/__init__.py
)

if (PYTHON_INSTALL_SUFFIX)
    install(
        DIRECTORY ${SONATA_PYTHON_BUILD}
        DESTINATION ${PYTHON_INSTALL_SUFFIX}
    )
else()
    message(WARNING "PYTHON_INSTALL_SUFFIX not set, Python bindings won't be installed")
endif()
