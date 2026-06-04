# 你画我猜绘画辅助工具

一款基于 Qt6 的桌面绘画辅助工具，可以将图片转换为线条画，并在屏幕指定区域自动绘制。

## 功能特性

- 🖼️ **图片加载**：支持 PNG、JPG、JPEG、BMP、GIF 等多种图片格式
- 🎨 **图片处理**：自动将彩色图片转换为黑白线条画
- 🎯 **画布选择**：通过鼠标框选屏幕任意区域作为绘画目标
- ✍️ **自动绘画**：在选定的画布区域自动绘制图片
- ⚡ **速度控制**：支持动态调整绘画速度
- ⌨️ **快捷键支持**：提供丰富的快捷键操作

## 环境要求

- Qt 6.x 或更高版本
- CMake 3.16 或更高版本
- C++17 兼容的编译器（GCC、Clang 或 MSVC）

## 构建步骤

详见 [BUILD.md](file:///workspace/BUILD.md)

### 中包含详细的多平台构建指南。

### 快速开始

```bash
# 克隆项目
git clone <repository_url>
cd DrawingHelper

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make

# 运行
./DrawingHelper
```

## 使用说明

1. **选择图片**：点击工具栏或菜单中的「选择图片」加载要绘制的图片

2. **解析图片**：程序会自动将图片转换为黑白线条画

3. **框选画布**：点击「框选画布」，用鼠标在屏幕上框选你画我猜的游戏画布，按回车确认

4. **开始绘画**：点击「开始绘画」，程序将自动在画布上绘制

5. **控制速度**：按 + 号增加速度，按 - 号减少速度

6. **终止绘画**：按 ESC 键停止绘画

## 项目结构

```
.
├── CMakeLists.txt          # CMake 构建配置
├── BUILD.md                 # 构建说明
├── LICENSE                    # GPLv3 许可证
├── README.md                 # 项目说明
└── src/
    ├── main.cpp             # 程序入口
    ├── mainwindow.h/cpp   # 主窗口界面
    └── imagecanvas.h/cpp  # 核心画布组件
```

## 核心组件

- [MainWindow](file:///workspace/src/mainwindow.h)：主窗口类，提供菜单栏、工具栏和状态栏界面
- [ImageCanvas](file:///workspace/src/imagecanvas.h)：核心画布组件，实现图片处理、画布选择和自动绘画功能

## 快捷键

| 功能 | 快捷键 |
|------|--------|
| 选择图片 | Ctrl+O |
| 框选画布 | C |
| 开始绘画 | S |
| 停止绘画 | X |
| 增加速度 | + |
| 减少速度 | - |
| 停止绘画 | ESC |

## 许可证

本项目采用 [GNU General Public License v3.0](file:///workspace/LICENSE) 许可证。

## 版权信息

版权所有 © 无忧。所有
