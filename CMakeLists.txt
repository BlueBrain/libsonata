cmake_minimum_required(VERSION 3.3)
cmake_policy(VERSION 3.3)

if(NOT SONATA_VERSION)
    execute_process(COMMAND git describe --tags
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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
option(SONATA_PYTHON "Build Python extensions" OFF)
option(SONATA_TESTS "Build tests" ON)
option(${PROJECT_NAME}_CXX_WARNINGS "Compile C++ with warnings as errors" ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SONATA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

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

if (EXTLIB_FROM_SUBMODULES)
    add_subdirectory(extlib EXCLUDE_FROM_ALL)
else()
    find_package(fmt REQUIRED)
    find_package(Catch2 REQUIRED)
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
    src/report_reader.cpp
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
)

target_compile_definitions(sonata_static
    PRIVATE FMT_HEADER_ONLY=1
)
target_include_directories(sonata_static
    PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
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

if (SONATA_TESTS)
    enable_testing()
    add_subdirectory(tests)

    if (ENABLE_COVERAGE)
        include(CodeCoverage)
        set(COVERAGE_LCOV_EXCLUDES '/usr/*' '${PROJECT_SOURCE_DIR}/include/*' '${PROJECT_SOURCE_DIR}/extlib/*')
        SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME coverage
            EXECUTABLE ctest
            DEPENDENCIES unittests
        )
    endif()
endif()

# =============================================================================
# Python bindings
# =============================================================================

if (SONATA_PYTHON)
    add_subdirectory(python)
endif()
