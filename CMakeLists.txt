cmake_minimum_required(VERSION 3.3)
cmake_policy(VERSION 3.3)

if(NOT SONATA_VERSION)
    execute_process(COMMAND git describe --tags
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE GIT_VERSION_FAILED
                    OUTPUT_VARIABLE GIT_PKG_VERSION_FULL
                    ERROR_VARIABLE GIT_VERSION_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(GIT_VERSION_FAILED)
      message(
        FATAL_ERROR
          "Could not retrieve version from command 'git describe --tags'\n"
          ${GIT_VERSION_ERROR})
    endif()

# keep last line of command output
    string(REPLACE "\n"
                   ";"
                   GIT_PKG_VERSION_FULL
                   "${GIT_PKG_VERSION_FULL}")
   list(GET GIT_PKG_VERSION_FULL -1 SONATA_VERSION)
endif()

# keep MAJOR.MINOR.PATCH (PATCH being optional)
string(REGEX
       REPLACE "v?([0-9]+\\.[0-9]+(\\.[0-9]+)?).*"
               "\\1"
               SONATA_VERSION
               "${SONATA_VERSION}")

# Get "major.minor" from string "major.minor.version"
string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${SONATA_VERSION})
set(SONATA_VERSION_ABI ${CMAKE_MATCH_1})

project(sonata VERSION ${SONATA_VERSION})

option(EXTLIB_FROM_SUBMODULES "Use Git submodules for header-only dependencies" OFF)
option(${PROJECT_NAME}_CXX_WARNINGS "Compile C++ with warnings as errors" ON)
option(REPORTS_ENABLE_MPI "Enable MPI-based execution for the reporting" OFF)

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

if(REPORTS_ENABLE_MPI)
    find_package(MPI REQUIRED)
    if (MPI_FOUND)
        # With MPI we prefer HDF5 parallel
        set(HDF5_PREFER_PARALLEL "ON")
    endif()

    find_package(HDF5)
    if (MPI_FOUND)
        if (HDF5_FOUND)
            if (HDF5_IS_PARALLEL)
                set(HAVE_MPI TRUE)
                # When MPI and HDF5 parallel are found, enable mpi in the report library
                add_definitions(-DHAVE_MPI)
                message(STATUS "Both MPI and HDF5 parallel found, using reporting parallel implementation")
            else()
                message(STATUS "MPI and HDF5 found, but no parallel IO support for HDF5, using reporting serial implementation")
            endif()
        endif()
    else()
        if (HDF5_FOUND)
            # Avoid NOTFOUND error
            set(MPI_CXX_LIBRARIES "")
            message(STATUS "HDF5 found, Using reporting serial implementation")
        else()
            message(FATAL_ERROR "No MPI or HDF5 found")
        endif()
    endif()

endif()

find_package(HDF5)
if (EXTLIB_FROM_SUBMODULES)
    add_subdirectory(extlib EXCLUDE_FROM_ALL)
    include(extlib/Catch2/contrib/Catch.cmake)
else()
    find_package(Catch2 REQUIRED)
    find_package(spdlog REQUIRED)
endif()

# =============================================================================
# Targets
# =============================================================================
add_subdirectory(src)

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

if (ENABLE_COVERAGE)
    include(CodeCoverage)
    set(COVERAGE_LCOV_EXCLUDES '/usr/*' '${PROJECT_SOURCE_DIR}/include/*' '${PROJECT_SOURCE_DIR}/extlib/*')
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME coverage
        EXECUTABLE ctest
        DEPENDENCIES unittests reports_unit_tests reports_integration_test
    )
endif()
