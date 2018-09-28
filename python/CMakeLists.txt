if (NOT SONATA_PYTHON)
    message(WARNING "SONATA_PYTHON not set, Python bindings won't be built")
    return()
endif()

add_subdirectory(pybind11 EXCLUDE_FROM_ALL)

add_library(sonata_python MODULE bindings.cpp)
set_target_properties(sonata_python PROPERTIES PREFIX "")
set_target_properties(sonata_python PROPERTIES OUTPUT_NAME "_sonata")

target_link_libraries(sonata_python
    PRIVATE sonata_shared
    PRIVATE pybind11::module
)

if (PYTHON_INSTALL_SUFFIX)
    install(
        TARGETS sonata_python LIBRARY
        DESTINATION ${PYTHON_INSTALL_SUFFIX}/libsonata
    )
    install(
        FILES __init__.py
        DESTINATION ${PYTHON_INSTALL_SUFFIX}/libsonata
    )
else()
    message(WARNING "PYTHON_INSTALL_SUFFIX not set, Python bindings won't be installed")
endif()
