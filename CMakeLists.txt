cmake_minimum_required(VERSION 3.3)
cmake_policy(VERSION 3.3)

file(STRINGS VERSION SONATA_VERSION)

# Get "major.minor" from string "major.minor.version"
string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${SONATA_VERSION})
set(SONATA_VERSION_ABI ${CMAKE_MATCH_1})

project(sonata VERSION ${SONATA_VERSION})

option(EXTLIB_FROM_SUBMODULES "Use Git submodules for header-only dependencies" OFF)
option(SONATA_PYTHON "Build Python extensions" OFF)
option(${PROJECT_NAME}_CXX_WARNINGS "Compile C++ with warnings as errors" ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)



set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SONATA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SONATA_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

if(SONATA_CXX_WARNINGS)
  set(SONATA_COMPILE_OPTIONS -Wall -Wextra -pedantic -Werror)
else()
  set(SONATA_COMPILE_OPTIONS -Wall -Wextra -pedantic)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_COVERAGE ON)
else()
    set(ENABLE_COVERAGE OFF)
endif()

# =============================================================================
# Dependencies
# =============================================================================

find_package(MPI)
if (MPI_FOUND)
    set(HDF5_PREFER_PARALLEL "ON")
    add_definitions(-DHAVE_MPI)
    include_directories (${MPI_INCLUDE_PATH})
endif()

find_package(HDF5)
if (HDF5_FOUND)
    if (HDF5_IS_PARALLEL)
        include_directories (${HDF5_INCLUDE_DIRS})
        set (HDF5_LIBS ${HDF5_LIBRARIES})
    else()
        message (STATUS "HDF5 found, but no parallel IO support was found, HDF5 support in replib will be disabled")
    endif()
endif()

if (EXTLIB_FROM_SUBMODULES)
    add_subdirectory(extlib EXCLUDE_FROM_ALL)
else()
    find_package(fmt REQUIRED)
    find_package(HighFive REQUIRED)
endif()

# =============================================================================
# Targets
# =============================================================================

set(SONATA_SRC
    src/common.cpp
    src/edge_index.cpp
    src/edges.cpp
    src/hdf5_mutex.cpp
    src/nodes.cpp
    src/population.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp
    )

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp
  )

add_library(sonata_shared SHARED ${SONATA_SRC})
add_library(sonata_static STATIC ${SONATA_SRC})

foreach(TARGET sonata_shared sonata_static)
    set_target_properties(${TARGET}
        PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            CXX_VISIBILITY_PRESET hidden
            OUTPUT_NAME "sonata"
    )
    target_include_directories(${TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_options(${TARGET}
        PRIVATE ${SONATA_COMPILE_OPTIONS}
    )
    if (ENABLE_COVERAGE)
        target_compile_options(${TARGET}
            PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage
        )
        target_link_libraries(${TARGET}
            PRIVATE gcov
        )
    endif()
endforeach(TARGET)

set_target_properties(sonata_shared
    PROPERTIES
        VERSION ${SONATA_VERSION}
        SOVERSION ${SONATA_VERSION_ABI}
)
target_compile_definitions(sonata_shared
    PUBLIC SONATA_DLL
    PRIVATE SONATA_DLL_EXPORTS
)
target_link_libraries(sonata_shared
    PRIVATE fmt::fmt-header-only
    PRIVATE HighFive
    ${MPI_CXX_LIBRARIES}
)

target_compile_definitions(sonata_static
    PRIVATE FMT_HEADER_ONLY=1
)
target_include_directories(sonata_static
    PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
)

include_directories(${SONATA_INCLUDE_DIR})
include_directories(${SONATA_SRC_DIR})
add_subdirectory(src/reports)

# =============================================================================
# Install
# =============================================================================

install(TARGETS sonata_shared sonata_static
    EXPORT sonata-targets
    LIBRARY
        DESTINATION lib
    ARCHIVE
        DESTINATION lib
)

install(DIRECTORY ${SONATA_INCLUDE_DIR}/bbp
    DESTINATION include
)

install(FILES CMake/sonata-config.cmake
    DESTINATION share/sonata/CMake
)

install(EXPORT sonata-targets
    DESTINATION share/sonata/CMake
    NAMESPACE sonata::
)

# =============================================================================
# Testing
# =============================================================================

enable_testing()
include(CMake/Catch.cmake)
add_subdirectory(tests)

if (ENABLE_COVERAGE)
    include(CodeCoverage)
    set(COVERAGE_GCOVR_EXCLUDES ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/extlib)
    setup_target_for_coverage_gcovr_xml(
        NAME coverage
        EXECUTABLE ctest
    )
endif()

# =============================================================================
# Python bindings
# =============================================================================

if (SONATA_PYTHON)
    add_subdirectory(python)
endif()
