cmake_minimum_required(VERSION 3.0)

set(SONATA_VERSION_MAJOR "0")
set(SONATA_VERSION_MINOR "0")
set(SONATA_VERSION_PATCH "0")

set(SONATA_VERSION "${SONATA_VERSION_MAJOR}.${SONATA_VERSION_MINOR}.${SONATA_VERSION_PATCH}")
set(SONATA_VERSION_ABI "${SONATA_VERSION_MAJOR}.${SONATA_VERSION_MINOR}")

project(sonata VERSION ${SONATA_VERSION})

option(EXTLIB_FROM_SUBMODULES "Use Git submodules for header-only dependencies" OFF)
option(SONATA_PYTHON "Build Python extensions" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SONATA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(SONATA_COMPILE_OPTIONS -Wall -Wextra -pedantic -Werror)

# =============================================================================
# Dependencies
# =============================================================================

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
    src/edges.cpp
    src/hdf5_mutex.cpp
    src/nodes.cpp
    src/population.cpp
)

add_library(sonata_obj OBJECT ${SONATA_SRC})
set_property(TARGET sonata_obj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(sonata_obj
    PRIVATE FMT_HEADER_ONLY=1
)
target_compile_options(sonata_obj
    PRIVATE ${SONATA_COMPILE_OPTIONS}
)
target_include_directories(sonata_obj
    PRIVATE ${SONATA_INCLUDE_DIR}
    PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(sonata_shared SHARED $<TARGET_OBJECTS:sonata_obj>)
set_target_properties(sonata_shared
    PROPERTIES
        OUTPUT_NAME "sonata"
        VERSION ${SONATA_VERSION}
        SOVERSION ${SONATA_VERSION_ABI}
)
target_include_directories(sonata_shared
    INTERFACE
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(sonata_shared
    PRIVATE HighFive
)

add_library(sonata_static STATIC $<TARGET_OBJECTS:sonata_obj>)
set_target_properties(sonata_static PROPERTIES OUTPUT_NAME "sonata")
target_include_directories(sonata_static
    INTERFACE
        $<BUILD_INTERFACE:${SONATA_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)

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
add_subdirectory(tests)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CodeCoverage)
    append_coverage_compiler_flags()
    set(COVERAGE_GCOVR_EXCLUDES ${PROJECT_SOURCE_DIR}/extlib)
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
