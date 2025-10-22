from mcp.server.fastmcp import FastMCP

# 初始化 MCP 服务器，并设置名称和版本
mcp = FastMCP("Hello World Server")

# 使用装饰器定义工具，函数文档字符串会自动作为工具描述
@mcp.tool()
def say_hello(name: str = "World") -> str:
    """一个简单的问候工具，向用户说Hello。

    Args:
        name: 要问候的对象名称，默认为'World'
    """
    return f"你好, {name}! 欢迎使用MCP 1.18.0 服务!"

# 运行服务器
if __name__ == "__main__":
    # 使用 stdio 传输，这是与 MCP 客户端（如 Claude Desktop）通信的标准方式
    mcp.run(transport="stdio")
