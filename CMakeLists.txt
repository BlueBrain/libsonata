cmake_minimum_required(VERSION 3.0)

project(libsonata VERSION 0.0.0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBSONATA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# =============================================================================
# Targets
# =============================================================================

set(LIBSONATA_SRC
  src/edges.cpp
)

add_library(sonata SHARED ${LIBSONATA_SRC})

target_include_directories(sonata PUBLIC ${LIBSONATA_INCLUDE_DIR})

# =============================================================================
# Install
# =============================================================================

install(TARGETS sonata DESTINATION lib)
install(DIRECTORY ${LIBSONATA_INCLUDE_DIR}/bbp DESTINATION include)

# =============================================================================
# Testing
# =============================================================================

enable_testing()
add_subdirectory(tests)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_xml(
        NAME coverage
        EXECUTABLE ctest
    )
endif()
