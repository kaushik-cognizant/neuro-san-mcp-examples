# C++ MCP Server

Located in directory: `cpp/`

A C++ implementation of the MCP server using:
- **[cpp-httplib](https://github.com/yhirose/cpp-httplib)** — header-only HTTP server
- **[nlohmann/json](https://github.com/nlohmann/json)** — header-only JSON library
- **CMake** with `FetchContent` (no manual dependency installation needed)


## Prerequisites

- CMake 3.15+
- A C++17-compatible compiler:
  - GCC 8+ / Clang 7+ (Linux / macOS)
  - MSVC 2019+ (Windows)
- Internet access on first build (to fetch dependencies via FetchContent)

## Build

```bash
cd cpp
cmake -B build
cmake --build build
```

On Windows (Visual Studio):
```cmd
cd cpp
cmake -B build
cmake --build build --config Release
```

## Start the server

```bash
./build/mcp_server
```

On Windows:
```cmd
build\Release\mcp_server.exe
```

The server will start at:
```
http://127.0.0.1:8000/mcp
```

## Tools exposed

| Tool name           | Description                                      |
|---------------------|--------------------------------------------------|
| `calculate_bmi`     | Calculate BMI given weight (kg) and height (m)   |
| `get_temperature`   | Get a mock temperature for a given city          |
| `get_greeting`      | Get a greeting message for a given name          |

## Verify the server

```bash
npx -y @modelcontextprotocol/inspector --cli http://127.0.0.1:8000/mcp --method tools/list
```

Expected output:
```json
{
  "tools": [
    { "name": "calculate_bmi",   ... },
    { "name": "get_temperature", ... },
    { "name": "get_greeting",    ... }
  ]
}
```
