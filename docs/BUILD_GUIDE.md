# 构建指南

## 环境要求

| 依赖 | 版本/说明 |
|------|-----------|
| Windows | 10/11 |
| CMake | 3.10+ |
| MSVC (Visual Studio) | 2017+ (C++17 支持) |
| OpenGL | 系统自带 (opengl32.dll) |

## 快速构建

```bash
cd D:/Projects/FerryManager
cmake --build build
```

CMake 会自动使用缓存的配置。如果是首次构建或需要重新配置：

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build
```

## 构建目标详解

### 配置阶段

CMakeLists.txt 使用 `file(GLOB IMGUI_SOURCES imgui/*.cpp)` 收集所有 ImGui 源文件。

共享的源文件 (`SOURCES` 变量)：
- `core/OrganismDefine.cpp` — 生物行为
- `core/EnvironmentDefine.cpp` — 环境行为
- `core/WorldDefine.cpp` — 世界逻辑
- `ui/SetupUI.cpp` — 设置界面
- `boids/boids.cpp` — Boids 算法
- `utils/img_utils.cpp` — 纹理工具
- `imgui/*.cpp` — ImGui 核心
- `imgui/backends/imgui_impl_win32.cpp` — Win32 后端
- `imgui/backends/imgui_impl_opengl3.cpp` — OpenGL 后端

### ecosim — 原始 GUI 应用

```bash
cmake --build build --target ecosim
```

**额外文件**：
- `test_main.cpp` (入口)
- `operator/MyOperator.h` + `operator/MyOperatorDefine.cpp` — 捕食/工厂
- `core/Animals.h` + `core/Plants.h` — 工厂子类
- `config/ConflgDefine.cpp` + `config/Config.h` — 配置
- `implot/implot.cpp` + `implot/implot_items.cpp` — 图表

**链接**：`opengl32 gdi32 dwmapi`

**构建后**：复制 `default_config.json` 到输出目录

**编译选项** (MSVC)：`/EHsc /utf-8`

**特点**：
- 纯色块 + 圆点绘制
- 固定物种图表 (Grass/Sheep/Wolf)
- Sheep 基因演化图表
- 能量热力图
- 位置查询面板
- 繁殖请求可视化

### ecosim2 — 增强版 GUI 应用

```bash
cmake --build build --target ecosim2
```

**额外文件**：使用 `imguMe/myui.cpp` 替代 `test_main.cpp`

**构建后**：
- 复制 `default_config.json` 到输出目录
- 复制整个 `assets/` 目录到输出目录

**特点**：
- PNG 纹理渲染生物和环境
- 动态所有物种的种群图表
- 手动添加生物功能（可选自定义基因）
- 更丰富的统计数据显示

### debug_sim — 终端批量模拟

```bash
cmake --build build --target debug_sim
```

**文件**：`debug_main.cpp` + 核心引擎（无 UI 相关）

**特点**：
- 纯终端输出，无 GUI 依赖
- 运行 1200 帧并打印每步统计数据
- 输出格式：`Step Plants Sheep Wolves P.min P.avg P.max S.min S.avg S.max W.min W.avg W.max P_req S_req W_req`
- 种群爆炸检测（>10000 自动终止）
- 前 30 帧 + 每 100 帧详细输出

### boids_test — Boids 独立测试

```bash
cmake --build build --target boids_test
```

**文件**：`boids/test.cpp` + `boids/boids.cpp` + ImGui

**特点**：
- 独立的 80×60 网格 Boids 模拟
- 可调节参数：cohesion, alignment, separation, vision, speed, dt
- 30 只 Wolf (红) + 50 只 Sheep (蓝)
- 环形边界（toroidal wrap）

### ecosim_lib — 共享库

```bash
cmake --build build --target ecosim_lib
```

**文件**：
- 所有核心引擎源文件
- 所有公开头文件（core/, config/, operator/, boids/, data/）

**构建后**：
- 生成 `ecosim_lib.dll` + `.lib`
- 复制所有公开头文件到 `build/includes/`
- 复制 `default_config.json` 到 DLL 目录
- 复制 `vendor/json.hpp`

**用于**：外部程序集成核心模拟引擎

## 项目包含路径

```cmake
target_include_directories(ecosim PRIVATE
    ${CMAKE_SOURCE_DIR}               # 项目根目录
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/operator
    ${CMAKE_SOURCE_DIR}/vendor        # json.hpp
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${CMAKE_SOURCE_DIR}/implot
    ${CMAKE_SOURCE_DIR}/stb_image
)
```

注意：源文件中使用 `#include "path/file.h"` 格式，`path` 相对于上述包含路径。

## 常见编译问题

### 1. 找不到 json.hpp

```
fatal error C1083: 无法打开包括文件: "json.hpp"
```

确保 `vendor/` 在包含路径中，且 `vendor/json.hpp` 存在。

### 2. 找不到 OpenGL

```
fatal error C1083: 无法打开包括文件: "GL/gl.h"
```

Windows 系统自带 OpenGL 头文件。如缺失，安装 Windows SDK。

### 3. 编译字符集问题

MSVC 需要 `/utf-8` 标志来正确处理源文件中的中文注释。

### 4. STB Image 重复定义

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```

只在 `utils/img_utils.cpp` 中定义 `STB_IMAGE_IMPLEMENTATION`。如果在多个 `.cpp` 中包含，会导致重复定义链接错误。

## 运行时文件查找

程序启动时会尝试多个路径查找配置文件：

```
default_config.json            # 从 exe 所在目录
../default_config.json         # 上级目录
../config/default_config.json
../../config/default_config.json
```

确保配置文件在可搜索路径中，或直接放在 exe 同级目录。CMake 的 POST_BUILD 命令会自动复制。

## 调试

### Visual Studio

```bash
cmake -B build -G "Visual Studio 17 2022"
# 在 VS 中打开 build/EcoSim.sln
```

### 终端调试输出

- `debug_sim` 目标直接输出到 stdout
- GUI 目标通过 `printf` 输出到调试控制台 (VS Output 窗口)

### 常见调试关注点

1. **负能量**：环境格能量 < 0 时会打印 `error` + 坐标
2. **种群爆炸**：debug_sim 会自动检测并终止
3. **工厂查不到物种**：`printf("77 %s\n", name)` 表示注册表中无此物种
