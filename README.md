# neuro-san-mcp-examples

This repository contains simple examples of MCP servers in various languages and connecting them with neuro-san.

All the example MCP servers are configured to serve at the `http://127.0.0.1:8000/mcp` URL and expose the same core tools. A sample neuro-san project is provided in the `neuro-san-server` directory, pre-configured to connect to the MCP server at that URL. A sample agent network call script is included, pre-configured to invoke the BMI calculation tool.

> Please read the [full MCP Guide for Neuro-SAN](https://github.com/cognizant-ai-lab/neuro-san-studio/blob/main/docs/user_guide.md#mcp-servers) for more info.

## Prerequisites

- Python 3.10+
- Java 21+ (only if using the Java MCP server)
- An **OpenAI API key** set in your environment:
  ```bash
  export OPENAI_API_KEY=your-key-here
  ```
  The neuro-san agent uses OpenAI (`gpt-5.2`) by default. You can change the model or provider in [`neuro-san-server/registries/llm_config.hocon`](neuro-san-server/registries/llm_config.hocon).

## Steps

1. Start any **one** of the MCP servers listed below (in the language you prefer).
2. Install the neuro-san server dependencies:
   ```bash
   pip install -r neuro-san-server/requirements.txt
   ```
3. Run the sample agent:
   ```bash
   python neuro-san-server/run_basic_mcp_agent.py
   ```

# MCP Servers

You can verify whether your MCP server is running using:

```bash
npx -y @modelcontextprotocol/inspector --cli http://127.0.0.1:8000/mcp --method tools/list
```

It should return the list of tools exposed by the server, for example:

```json
{
  "tools": [
    {
      "name": "calculate_bmi",
      "description": "Calculate BMI given weight in kg and height in meters",
      "inputSchema": {
        "type": "object",
        "properties": {
          "weight": { "type": "number" },
          "height": { "type": "number" }
        },
        "required": ["weight", "height"]
      }
    }
  ]
}
```

The available example MCP servers are listed below.

## Python

Located in directory: `python/`

Install dependencies:
```bash
pip install -r python/requirements.txt
```

Start the server:
```bash
python python/start_mcp_server.py
```

Tools exposed: `calculate_bmi`

## Java

Located in directory: `java/`

Start the server:
```bash
cd java && mvn spring-boot:run
```

Tools exposed: `calculateBmi`, `getTemperature`, `getGreeting`

See [`java/README.md`](java/README.md) for more details.

## C++

Located in directory: `cpp/`

Build the server:
```bash
cd cpp
cmake -B build && cmake --build build
```

Start the server:
```bash
./build/mcp_server
```

Tools exposed: `calculate_bmi`, `get_temperature`, `get_greeting`

See `cpp/README.md` for more details.
