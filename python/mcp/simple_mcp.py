#!/usr/bin/env python3
import asyncio
from mcp.server.models import InitializationOptions
from mcp.server import NotificationOptions, Server
from mcp.server.stdio import stdio_server
import mcp.types as types

# 创建标准服务器实例
server = Server("greeting-server")

@server.list_tools()
async def handle_list_tools() -> list[types.Tool]:
    """列出所有可用工具"""
    return [
        types.Tool(
            name="say_hello",
            description="向指定的人打招呼",
            inputSchema={
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "要打招呼的人的姓名"
                    }
                },
                "required": []
            }
        ),
        types.Tool(
            name="say_hello_with_time",
            description="根据时间打招呼",
            inputSchema={
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "要打招呼的人的姓名"
                    },
                    "time_of_day": {
                        "type": "string",
                        "description": "时间：早上、下午、晚上",
                        "enum": ["早上", "下午", "晚上"]
                    }
                },
                "required": ["name"]
            }
        )
    ]

@server.call_tool()
async def handle_call_tool(name: str, arguments: dict) -> list[types.TextContent]:
    """处理工具调用"""
    if name == "say_hello":
        user_name = arguments.get("name", "朋友")
        greeting = f"你好，{user_name}！欢迎使用 MCP 服务！"
        return [types.TextContent(type="text", text=greeting)]

    elif name == "say_hello_with_time":
        user_name = arguments.get("name", "朋友")
        time_of_day = arguments.get("time_of_day", "现在")

        time_greetings = {
            "早上": "早安",
            "下午": "下午好",
            "晚上": "晚上好"
        }
        time_greeting = time_greetings.get(time_of_day, "你好")
        greeting = f"{time_greeting}，{user_name}！很高兴为您服务。"
        return [types.TextContent(type="text", text=greeting)]

    else:
        raise ValueError(f"未知工具: {name}")

async def main():
    """主函数，启动服务器"""
    async with stdio_server() as (read_stream, write_stream):
        await server.run(
            read_stream,
            write_stream,
            InitializationOptions(
                server_name="greeting-server",
                server_version="1.0.0",
                capabilities=server.get_capabilities(
                    notification_options=NotificationOptions(),
                    experimental_capabilities={}
                )
            )
        )

if __name__ == "__main__":
    asyncio.run(main())
