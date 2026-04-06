cmake_minimum_required(VERSION 3.15)
project(cpp_mcp_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch dependencies ────────────────────────────────────────────────────────

include(FetchContent)

# Header-only HTTP server
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.1
)

# Header-only JSON library
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(httplib nlohmann_json)

# ── Build target ──────────────────────────────────────────────────────────────

add_executable(mcp_server main.cpp)

target_link_libraries(mcp_server
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
)

# Enable warnings
if (MSVC)
    target_compile_options(mcp_server PRIVATE /W4)
else()
    target_compile_options(mcp_server PRIVATE -Wall -Wextra -Wpedantic)
endif()
