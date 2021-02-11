add_subdirectory(HighFive)
option(FMT_INSTALL "Install fmt for static library" ON)
add_subdirectory(fmt)
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE
    nlohmann
)

if (SONATA_TESTS)
    add_subdirectory(Catch2)
endif()
