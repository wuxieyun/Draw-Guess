# 你画我猜绘画辅助工具 v0.0.1 - 构建说明

## 环境要求

- Qt 6.x 或更高版本
- CMake 3.16 或更高版本
- C++17 兼容的编译器（GCC, Clang 或 MSVC）

## 构建步骤

### Linux/macOS

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make

# 运行
./DrawingHelper
```

### Windows (MinGW)

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake -G "MinGW Makefiles" ..

# 编译
mingw32-make

# 运行
DrawingHelper.exe
```

### Windows (Visual Studio)

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 打开解决方案
start DrawingHelper.sln

# 在 Visual Studio 中编译并运行
```

## 使用说明

1. **选择图片**：点击工具栏或菜单中的「选择图片」加载要绘制的图片
2. **解析图片**：程序会自动将图片转换为黑白线条画
3. **框选画布**：点击「框选画布」，用鼠标在屏幕上框选你画我猜的游戏画布，按回车确认
4. **开始绘画**：点击「开始绘画」，程序将自动在画布上绘制
5. **控制速度**：按 + 号增加速度，按 - 号减少速度
6. **终止绘画**：按 ESC 键停止绘画

## 版权信息

版权：无忧。所有
