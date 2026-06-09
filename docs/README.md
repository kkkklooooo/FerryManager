# FerryManager (EcoSim) — 项目文档

## 项目概述

FerryManager (内部代号 EcoSim) 是一个**生态系统模拟器**，基于 C++17 开发，使用 Win32 + OpenGL 渲染，ImGui 构建交互界面。

模拟的核心是**能量流动**：环境提供能量 → 植物吸收 → 草食动物吃植物 → 肉食动物吃草食动物。所有生物通过遗传算法 (GA) 和 Boids 集群算法进行繁殖、移动和演化。

## 技术栈

| 技术 | 用途 |
|------|------|
| C++17 | 主语言 |
| Win32 API | 窗口管理 |
| OpenGL 1.x | 渲染 |
| ImGui | 即时模式 GUI |
| ImPlot | 数据图表绘制 |
| nlohmann/json | JSON 解析 |
| STB Image | 纹理加载 |
| Eigen | 线性代数（备用） |
| CMake 3.10+ | 构建系统 |

## 构建目标

| 目标 | 类型 | 说明 |
|------|------|------|
| `ecosim` | GUI 应用 | 原始版本，纯色块绘制 |
| `ecosim2` | GUI 应用 | 增强版，PNG 纹理渲染 + 高级图表 |
| `debug_sim` | 终端程序 | 无 GUI，用于快速批量模拟和参数调优 |
| `boids_test` | GUI 应用 | 独立的 Boids 集群算法可视化测试 |
| `ecosim_lib` | 共享库(DLL) | 核心模拟逻辑导出为动态库 |

## 文档索引

- **[架构设计](ARCHITECTURE.md)** — 总体架构、设计模式、数据流、类层次结构
- **[模块说明](MODULES.md)** — 各模块目录的职责、文件清单、核心 API
- **[模拟机制](SIMULATION.md)** — 能量模型、捕食系统、繁殖逻辑、Boids 集群、遗传算法
- **[构建指南](BUILD_GUIDE.md)** — 编译环境、CMake 配置、所有构建目标
- **[配置系统](CONFIGURATION.md)** — 引擎参数、物种配置、JSON 结构
- **[平衡调优指南](BALANCE_GUIDE.md)** — 参数调优工作流、常见问题、能量平衡策略
- **[OOP 最佳实践](OOP_BEST_PRACTICES.md)** — 已存在的重构建议和代码规范记录

## 快速开始

```bash
cd D:/Projects/FerryManager
cmake --build build
```

编译后在 `build/` 目录下运行：

```bash
# 完整 GUI 模拟
./build/ecosim.exe

# 增强版 GUI (PNG 纹理)
./build/ecosim2.exe

# 终端批量模拟（参数调优）
./build/debug_sim.exe

# Boids 算法独立测试
./build/boids_test.exe
```

## 项目目录结构

```
FerryManager/
├── core/               # 核心模拟引擎
│   ├── Registry.h      # 枚举定义 (OrganismType, Weather 等)
│   ├── Organism.h      # 生物基类、可繁殖类、植物、动物
│   ├── OrganismDefine.cpp  # 生物行为实现
│   ├── Animals.h       # UserAnimal 工厂
│   ├── Plants.h        # UserPlant 工厂
│   ├── Environment.h   # 环境格类 (GressLand, Water)
│   ├── EnvironmentDefine.cpp  # 环境行为实现
│   ├── World.h         # 世界类（单例，控制器）
│   └── WorldDefine.cpp # 世界逻辑实现
├── config/             # 配置系统
│   ├── Config.h        # 引擎参数常量 / JSON 结构体定义
│   ├── ConflgDefine.cpp    # 配置加载和校验逻辑
│   └── default_config.json # 默认物种配置
├── boids/              # Boids 集群算法
│   ├── boids.h         # 粒子创建、邻居查找、合力计算
│   ├── boids.cpp       # Boids 算法实现
│   ├── Genes.h         # 基因结构体 (Particle, Genes)
│   ├── GA.h            # 遗传算法（基因融合）
│   └── test.cpp        # Boids 独立可视化测试入口
├── operator/           # 工厂/操作模式
│   ├── MyOperator.h    # 工厂注册 + 捕食交互
│   └── MyOperatorDefine.cpp  # 操作实现
├── ui/                 # UI 层
│   ├── SetupUI.h       # 启动配置界面
│   └── SetupUI.cpp     # 设置 UI 实现
├── imgui/              # Dear ImGui 库
├── implot/             # ImPlot 图表库
├── imguMe/             # 增强版 UI (PNG 纹理渲染)
│   └── myui.cpp        # 增强版 UI 入口
├── data/               # 游戏存档数据
│   ├── game_data.json  # 世界存档索引
│   └── game_struct.h   # 数据结构定义
├── assets/             # PNG 纹理资源
├── utils/              # 工具
│   ├── img_utils.h     # 纹理加载
│   └── img_utils.cpp   # STB Image 封装
├── vendor/             # 第三方库
│   └── json.hpp        # nlohmann/json
├── modules/Eigen/      # Eigen 线性代数库
├── stb_image/          # STB Image 库
├── MAKE_IMAGE/         # 图像管理
├── docs/               # 项目文档
├── test_main.cpp       # ecosim 主入口
├── debug_main.cpp      # debug_sim 主入口
└── CMakeLists.txt      # CMake 构建配置
```

## 核心概念速览

### 能量流动

```
太阳光 → GressLand 环境能量 (+0.8/帧)
    ↓ (plant_absorb_rate × cellEnergy)
植物吸收 → 植物能量增加
    ↓ (被动物捕食)
动物捕食 → 动物能量增加
    ↓ (动物互相捕食)
顶级捕食者
    ↓ (死亡)
尸体能量回归环境 (× loss_rate)
```

### 类继承关系

```
Organism (基类: 能量、位置、速度、活跃状态、Step)
  ├── Reproducable (可繁殖: 繁殖阈值、消耗、半径、食谱、天敌)
  │     ├── Plant (植物: 从环境吸收能量)
  │     │     └── UserPlant (工厂创建)
  │     └── Animal (动物: Boids 移动、捕食)
  │           └── UserAnimal (工厂创建)
  └── Environment (环境格: 能量、容纳生物)
        ├── GressLand (草地: 阳光增益)
        └── Water (水域)
```

### 模拟主循环

```
World::Update()
  1. 清空环境生物列表
  2. RemoveDeadOrganisms() — 删除死亡生物
  3. Environment::Update() — 更新环境能量
  4. 遍历生物: EnergyExchange(吸收) → Step(移动/代谢)
  5. 交互检测: PredationOrFuck(捕食或繁殖)
  6. Reproduce() — 统一创建子代
```

## 版本历史

- **v0.1** — 基础生态系统模拟、Boids 集群、遗传算法、ImGui 界面
- 最近提交: Boids 算法重构、PNG 纹理支持、参数调优工具
