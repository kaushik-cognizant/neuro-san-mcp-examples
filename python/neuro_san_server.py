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


# @mcp.tool()
# def add(a: int, b: int) -> int:
#     """Add two numbers"""
#     return a + b

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
