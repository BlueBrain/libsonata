set(SONATA_PYTHON_BUILD ${CMAKE_CURRENT_BINARY_DIR}/libsonata)

add_library(sonata_mpi INTERFACE)

if(DEFINED SONATA_MPI4PY_INCLUDE_DIR)
    find_package(MPI REQUIRED)

    target_include_directories(sonata_mpi INTERFACE ${SONATA_MPI4PY_INCLUDE_DIR})
    target_link_libraries(sonata_mpi INTERFACE MPI::MPI_C)
    target_compile_definitions(sonata_mpi INTERFACE SONATA_HAS_MPI=1)
endif()

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
    PRIVATE sonata_mpi
)
