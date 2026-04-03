# Minimal MCP Server for Neuro-San

This guide walks through building, verifying, and wiring a minimal MCP server into a Neuro-San agent network.

---

## 1. The Server Script

Save the following as `neuro_san_server.py`:

```python
"""
MCP server compatible with Neuro-san's MCP protocol version 2025-06-18.

Transport: JSON-RPC 2.0 over HTTP (non-streaming).
Each request returns a single JSON-RPC payload — no SSE/chunked streaming.
Sessions are not maintained between requests (stateless), which allows
horizontal scaling without sticky routing.
"""

from mcp.server.fastmcp import FastMCP
from starlette.middleware.cors import CORSMiddleware

# json_response=True  → responses are single JSON-RPC payloads, not SSE streams.
# stateless_http=True → no server-side session state; each request is self-contained.
# Both settings are required for Neuro-san MCP protocol version 2025-06-18 compatibility.
mcp = FastMCP("Neuro-San Server", stateless_http=True, json_response=True)

app = mcp.streamable_http_app()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


@mcp.tool()
def calculate_bmi(weight: float, height: float) -> float:
    """Calculate BMI given weight in kg and height in meters"""
    return weight / (height ** 2)


@mcp.resource("greeting://{name}")
def get_greeting(name: str) -> str:
    """Get a personalized greeting"""
    return f"Hello, {name}!"


@mcp.prompt()
def greet_user(name: str, style: str = "friendly") -> str:
    """Generate a greeting prompt"""
    return f"Write a {style} greeting for someone named {name}."


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
```

### Running the server

```bash
pip install "mcp[cli]" uvicorn starlette
python neuro_san_server.py
```

The server starts on `http://127.0.0.1:8000`. The MCP endpoint is available at `http://127.0.0.1:8000/mcp`.

### Why HTTP and not stdio

MCP supports two main transports: **stdio** and **HTTP**. Neuro-san exclusively uses **HTTP JSON-RPC 2.0** — it discovers and calls tools by sending POST requests to a URL registered in `mcp_info.hocon`. stdio transport pipes messages through a process's stdin/stdout, which means:

- The MCP client must **spawn and own** the server process — Neuro-san does not do this.
- It is limited to a **single client** and is not addressable over a network.
- It cannot be shared between multiple agents or scaled horizontally.

Because Neuro-san locates MCP servers by URL, the server **must** be a running HTTP process.

---

## 2. Verifying with the MCP Inspector

Before wiring the server into Neuro-san, verify it works correctly using the official MCP Inspector. With the server already running, open a second terminal and run:

```bash
npx -y @modelcontextprotocol/inspector --cli http://127.0.0.1:8000/mcp --method tools/list
```

If the server is running correctly, you should see a response like this:

```json
{
  "tools": [
    {
      "name": "calculate_bmi",
      "description": "Calculate BMI given weight in kg and height in meters",
      "inputSchema": {
        "type": "object",
        "properties": {
          "weight": {
            "title": "Weight",
            "type": "number"
          },
          "height": {
            "title": "Height",
            "type": "number"
          }
        },
        "required": [
          "weight",
          "height"
        ],
        "title": "calculate_bmiArguments"
      },
      "outputSchema": {
        "type": "object",
        "properties": {
          "result": {
            "title": "Result",
            "type": "number"
          }
        },
        "required": [
          "result"
        ],
        "title": "calculate_bmiOutput"
      }
    }
  ]
}
```

> **Tip:** If the inspector reports a protocol mismatch, ensure both `stateless_http=True` and `json_response=True` are set on the `FastMCP` instance. Without `json_response=True`, FastMCP defaults to SSE streaming, which is incompatible with Neuro-san's transport expectations.

---

## 3. Creating the Agent File

Neuro-san agent networks are defined in HOCON files inside the `registries/` directory. Create `registries/basic_agent.hocon`:

```hocon
{
    "metadata": {
        "description": "Simple agent that demonstrates tool use with a custom tool that calculates BMI.",
        "tags": ["mcp"],
        "sample_queries": [
            "90kg and 1.6m. calculate BMI",
        ]
    },

    "llm_config": {
        "class": "openai",
        "model_name": "gpt-4o-mini",
    },

    "tools": [
        {
            "name": "HelloWorldAgent",
            "function": {
                # The description acts as an initial prompt.
                "description": """I greet people"""
            },
            "instructions": """You are an agent that greets people. When you
            receive a request, respond with a friendly greeting message. Use your tools to calculate BMI""",
            "tools": ["http://127.0.0.1:8000/mcp"]
        },
    ]
}
```

**Key points:**

- The `"tools"` array inside an agent entry accepts **MCP server URLs** directly — Neuro-san resolves them at startup via `mcp_info.hocon`.
- `"description"` in `"function"` acts as the agent's initial system prompt when it is invoked as a sub-tool by a parent agent.
- `"instructions"` is the full system prompt for this agent's LLM calls.

---

## 4. Registering the Server in `mcp_info.hocon`

Neuro-san loads all MCP servers listed in `mcp/mcp_info.hocon` at startup and makes their tools available to agents. Add your server's endpoint:

```hocon
{
    "http://127.0.0.1:8000/mcp": {},

    # Optionally restrict which tools are loaded from this server:
    # "http://127.0.0.1:8000/mcp": {
    #     "tools": ["calculate_bmi"]
    # },

    # Remote servers with auth follow the same pattern:
    # "https://mcp.example.com/mcp": {
    #     "http_headers": {
    #         "Authorization": "Bearer " ${MY_API_TOKEN}
    #     }
    # },
}
```

An empty `{}` value means **load all tools** from that server. The `"tools"` key accepts a list of tool names if you want to expose only a subset.

> **Note:** `mcp_info.hocon` is loaded from the path set in the `MCP_SERVERS_INFO_FILE` environment variable, which `run.py` sets automatically to `mcp/mcp_info.hocon` relative to the studio root.

---

## 5. Invoking the Network from the Studio

With the MCP server running and both config files in place, start the Neuro-san studio from the `neuro-san-studio/` directory:

```bash
cd neuro-san-studio
python run.py
```

This starts:
- The **Neuro-san gRPC/HTTP server** (default port `8080`) — this is the agent orchestration backend, separate from your MCP server.
- The **nsflow web client** (default port `4173`) — the browser UI for interacting with agent networks.

Open the nsflow UI in your browser (`http://localhost:4173`), select **basic_agent** from the network list, and send a query such as:

```
90kg and 1.6m. calculate BMI
```

Neuro-san's orchestration layer will route the request to `HelloWorldAgent`, which will call the `calculate_bmi` tool on your MCP server at `http://127.0.0.1:8000/mcp`, and return the result.

### Ports at a glance

| Service | Default port | Purpose |
|---|---|---|
| MCP server (`neuro_san_server.py`) | `8000` | Exposes tools via JSON-RPC 2.0 HTTP |
| Neuro-san server (`run.py`) | `8080` | Agent orchestration backend |
| nsflow web client | `4173` | Browser UI |
