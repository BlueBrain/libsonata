cmake_minimum_required(VERSION 3.0)

set(LIBSONATA_VERSION 0.0.0)

project(libsonata VERSION ${LIBSONATA_VERSION})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBSONATA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(LIBSONATA_COMPILE_OPTIONS -Wall -Wextra -pedantic -Werror)

# =============================================================================
# Dependencies
# =============================================================================

add_subdirectory(extlib EXCLUDE_FROM_ALL)

# =============================================================================
# Targets
# =============================================================================

set(LIBSONATA_SRC
    src/common.cpp
    src/edges.cpp
    src/nodes.cpp
    src/population.cpp
)

add_library(sonata_obj OBJECT ${LIBSONATA_SRC})
set_property(TARGET sonata_obj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(sonata_obj
    PUBLIC FMT_HEADER_ONLY=1
)
target_compile_options(sonata_obj
    PUBLIC ${LIBSONATA_COMPILE_OPTIONS}
)
target_include_directories(sonata_obj
    PRIVATE ${LIBSONATA_INCLUDE_DIR}
    PRIVATE $<TARGET_PROPERTY:fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(sonata_shared SHARED $<TARGET_OBJECTS:sonata_obj>)
set_target_properties(sonata_shared PROPERTIES OUTPUT_NAME "sonata")
target_include_directories(sonata_shared
    PUBLIC ${LIBSONATA_INCLUDE_DIR}
)
target_link_libraries(sonata_shared
    PRIVATE HighFive
)

add_library(sonata_static STATIC $<TARGET_OBJECTS:sonata_obj>)
set_target_properties(sonata_static PROPERTIES OUTPUT_NAME "sonata")
target_include_directories(sonata_static
    PUBLIC ${LIBSONATA_INCLUDE_DIR}
)

# =============================================================================
# Install
# =============================================================================

install(TARGETS sonata_shared DESTINATION lib OPTIONAL)
install(TARGETS sonata_static DESTINATION lib OPTIONAL)
install(DIRECTORY ${LIBSONATA_INCLUDE_DIR}/bbp DESTINATION include OPTIONAL)

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

add_subdirectory(python)
