// C++ MCP Server using cpp-httplib and nlohmann/json
// Serves at http://127.0.0.1:8000/mcp
// Implements the MCP Streamable HTTP transport (JSON-RPC 2.0)

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <stdexcept>

using json = nlohmann::json;

// ─── Tool implementations ──────────────────────────────────────────────────

double compute_bmi(double weight_kg, double height_m) {
    if (height_m <= 0) throw std::invalid_argument("Height must be greater than 0");
    return weight_kg / (height_m * height_m);
}

std::string mock_temperature(const std::string& city) {
    // Mock temperatures – replace with a real weather API if desired
    static const std::unordered_map<std::string, std::string> temps = {
        {"london",    "15°C"}, {"new york", "22°C"},
        {"tokyo",     "25°C"}, {"delhi",    "35°C"},
        {"sydney",    "20°C"}
    };
    std::string lower = city;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto it = temps.find(lower);
    return "Temperature in " + city + ": " + (it != temps.end() ? it->second : "28°C") + " (mock)";
}

std::string make_greeting(const std::string& name) {
    return "Hello, " + name + "! Greetings from the C++ MCP server.";
}

// ─── Tool metadata ─────────────────────────────────────────────────────────

json tools_list() {
    return json::array({
        {
            {"name", "calculate_bmi"},
            {"description", "Calculate BMI given weight in kg and height in meters"},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {
                    {"weight", {{"type", "number"}, {"description", "Weight in kilograms"}}},
                    {"height", {{"type", "number"}, {"description", "Height in meters"}}}
                }},
                {"required", json::array({"weight", "height"})}
            }}
        },
        {
            {"name", "get_temperature"},
            {"description", "Get a (mock) temperature for a given city"},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {
                    {"city", {{"type", "string"}, {"description", "Name of the city"}}}
                }},
                {"required", json::array({"city"})}
            }}
        },
        {
            {"name", "get_greeting"},
            {"description", "Get a greeting message for a given name"},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}, {"description", "Name to greet"}}}
                }},
                {"required", json::array({"name"})}
            }}
        }
    });
}

// ─── JSON-RPC handlers ─────────────────────────────────────────────────────

json handle_initialize(const json& req) {
    return {
        {"jsonrpc", "2.0"},
        {"id", req["id"]},
        {"result", {
            {"protocolVersion", "2024-11-05"},
            {"capabilities", {{"tools", json::object()}}},
            {"serverInfo", {{"name", "cpp-mcp-server"}, {"version", "1.0.0"}}}
        }}
    };
}

json handle_tools_list(const json& req) {
    return {
        {"jsonrpc", "2.0"},
        {"id", req["id"]},
        {"result", {{"tools", tools_list()}}}
    };
}

json handle_tools_call(const json& req) {
    const std::string tool_name = req["params"]["name"];
    const json args = req["params"].value("arguments", json::object());

    std::string result_text;

    try {
        if (tool_name == "calculate_bmi") {
            double weight = args.at("weight").get<double>();
            double height = args.at("height").get<double>();
            double bmi = compute_bmi(weight, height);
            // Round to 2 decimal places for readability
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.2f", bmi);
            result_text = "BMI: " + std::string(buf);

        } else if (tool_name == "get_temperature") {
            result_text = mock_temperature(args.at("city").get<std::string>());

        } else if (tool_name == "get_greeting") {
            result_text = make_greeting(args.at("name").get<std::string>());

        } else {
            return {
                {"jsonrpc", "2.0"},
                {"id", req["id"]},
                {"error", {{"code", -32601}, {"message", "Tool not found: " + tool_name}}}
            };
        }
    } catch (const std::exception& e) {
        return {
            {"jsonrpc", "2.0"},
            {"id", req["id"]},
            {"error", {{"code", -32602}, {"message", std::string("Invalid params: ") + e.what()}}}
        };
    }

    return {
        {"jsonrpc", "2.0"},
        {"id", req["id"]},
        {"result", {
            {"content", json::array({{
                {"type", "text"},
                {"text", result_text}
            }})}
        }}
    };
}

json dispatch(const json& req) {
    const std::string method = req.value("method", "");

    if (method == "initialize")   return handle_initialize(req);
    if (method == "tools/list")   return handle_tools_list(req);
    if (method == "tools/call")   return handle_tools_call(req);

    return {
        {"jsonrpc", "2.0"},
        {"id", req.value("id", nullptr)},
        {"error", {{"code", -32601}, {"message", "Method not found: " + method}}}
    };
}

// ─── Entry point ───────────────────────────────────────────────────────────

int main() {
    httplib::Server svr;

    svr.Post("/mcp", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        try {
            const json body = json::parse(req.body);
            const json response = dispatch(body);
            res.set_content(response.dump(), "application/json");
        } catch (const json::parse_error& e) {
            const json error = {
                {"jsonrpc", "2.0"},
                {"id",      nullptr},
                {"error",   {{"code", -32700}, {"message", std::string("Parse error: ") + e.what()}}}
            };
            res.status = 400;
            res.set_content(error.dump(), "application/json");
        }
    });

    // OPTIONS pre-flight for CORS
    svr.Options("/mcp", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    const std::string host = "127.0.0.1";
    const int         port = 8000;
    std::cout << "C++ MCP server running at http://" << host << ":" << port << "/mcp\n";
    std::cout << "Tools: calculate_bmi, get_temperature, get_greeting\n";

    if (!svr.listen(host.c_str(), port)) {
        std::cerr << "Failed to start server on port " << port << "\n";
        return 1;
    }
    return 0;
}
