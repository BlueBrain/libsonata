if (NOT SONATA_PYTHON)
    message(WARNING "SONATA_PYTHON not set, Python bindings won't be built")
    return()
endif()

set(LIBSONATA_PYTHON_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sonata)

add_subdirectory(pybind11 EXCLUDE_FROM_ALL)

add_library(sonata_python MODULE bindings.cpp)
set_target_properties(sonata_python PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_sonata"
    LIBRARY_OUTPUT_DIRECTORY ${LIBSONATA_PYTHON_BUILD}
)

target_link_libraries(sonata_python
    PRIVATE sonata_static
    PRIVATE HighFive
    PRIVATE fmt-header-only
    PRIVATE pybind11::module
)

configure_file(
    sonata/__init__.py.in
    ${LIBSONATA_PYTHON_BUILD}/__init__.py
)

if (PYTHON_INSTALL_SUFFIX)
    install(
        DIRECTORY ${LIBSONATA_PYTHON_BUILD}
        DESTINATION ${PYTHON_INSTALL_SUFFIX}
        OPTIONAL
    )
else()
    message(WARNING "PYTHON_INSTALL_SUFFIX not set, Python bindings won't be installed")
endif()
