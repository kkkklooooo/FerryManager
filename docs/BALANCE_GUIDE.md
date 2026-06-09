# 平衡调优指南

## 调优工具

### debug_sim — 批量模拟

```bash
cmake --build build && cd build && ./debug_sim.exe
```

输出 16 列制表数据：

```
Step   Plants  Sheep  Wolves  P.min  P.avg  P.max  S.min  S.avg  S.max  W.min  W.avg  W.max  P_req  S_req  W_req
-----  ------  -----  ------  -----  -----  -----  -----  -----  -----  -----  -----  -----  ------  -----  -----
```

| 列 | 说明 |
|----|------|
| Step | 帧号 |
| Plants / Sheep / Wolves | 三个核心物种的个体数 |
| P.min / P.avg / P.max | 植物能量最小/平均/最大 |
| S.min / S.avg / S.max | 羊能量统计 |
| W.min / W.avg / W.max | 狼能量统计 |
| P_req / S_req / W_req | 本帧繁殖请求数 |

### ecosim GUI — 实时可视化

- **种群图表** (`Population Chart`)：实时折线图 vs 帧数
- **基因图表** (`Sheep Gene Stats`)：Sheep 6 个基因的演化趋势
- **生物列表**：每个个体的能量、位置
- **位置查询**：鼠标悬停或输入坐标查看详情

## 能量平衡核心公式

### 系统稳态条件

要使种群稳定（不爆炸不灭绝），需满足：

```
总能量输入 ≈ 总能量消耗
```

即：

```
环境增益 Σ(每个格子的太阳能输入)
    ≈
生物代谢消耗 Σ(每个生物的 step_cost × overlay_factor)
    +
生物死亡浪费 (未被完全回收的尸体能量)
```

### 关键平衡关系

| 关系 | 公式 | 调优参数 |
|------|------|---------|
| 植物净收益 | `吸收 - 代谢 - 拥挤惩罚` | `plant_absorb_rate`, `step_energy_cost`, `overlay_param` |
| 草食动物净收益 | `捕食获取 - 代谢 - 拥挤惩罚 - 被捕食损失` | `animal_absorb_rate`, `loss_rate`, `eat_intrval_max` |
| 捕食者净收益 | `捕食获取 - 代谢` | `max_energy`, `energy_rate`, `killChance` |

## 常见问题诊断与修复

### 1. 植物全部灭绝（Plants → 0，所有动物随后灭绝）

**症状**：
```
Step  Plants  Sheep  Wolves
   5      56     30       8
  20      23      5       8
  30       0      0       0
```

**原因分析**：
- 植物能量消耗 > 吸收 → 无法维持 → 全死
- 常见触发条件：
  - `step_energy_cost` 过高
  - 太阳能增益不足
  - 初始植物太少

**修复方案**：
1. 降低 `PlantConfig.step_energy_cost` (如 0.2 → 0.1)
2. 增加 `GressLand::Update()` 中的太阳能增益 (0.8 → 1.2)
3. 增加初始植物数量 (World 构造函数中 `plantsPerSeed` 8 → 15)

### 2. 种群爆炸（所有物种数量激增）

**症状**：
```
Step  Plants  Sheep  Wolves
  50      56      30       8
 200     500     200      50
 400    2000     800     200
 800  10000+ crash
```

**原因分析**：
- 能量产生 > 消耗 → 持续增长
- 常见触发条件：
  - `step_energy_cost` 过低
  - `max_organisms_per_cell` 过高
  - `overlay_param` 设置不当 → 拥挤惩罚太弱

**修复方案**：
1. 提高 `step_energy_cost`
2. 降低 `max_organisms_per_cell` (4 → 3)
3. 调整 `overlay_param`：使其接近典型密度值，增强惩罚效果
4. 增加 `eat_intrval_max`（降低动物进食频率）

### 3. 捕食者灭绝（Wolf → 0，Sheep 和 Plants 稳定）

**症状**：
```
Step  Plants  Sheep  Wolves
  50      80      40       8
 100     120      60       5
 150     150      80       2
 200     180     100       0
```

**原因分析**：
- 捕食者能量获取不足以维持种群
- 常见触发条件：
  - `animal_absorb_rate` 过低
  - 捕食者 `max_energy` 过高 → 需要大量能量才吃饱
  - `eat_intrval_max` 过大 → 进食频率太低
  - 初始捕食者太少/太分散 → 找不到配偶

**修复方案**：
1. 提高 `animal_absorb_rate` (0.30 → 0.50)
2. 降低捕食者的 `eat_intrval_max`
3. 在中心区域更密集地初始化捕食者
4. 增加捕食者 `reproduce_original_energy`

### 4. 猎物灭绝（Sheep → 0，Wolf 随后灭绝）

**症状**：
```
Step  Plants  Sheep  Wolves
  50      80      30       8
 100     120      15      15
 150     200       5      25
 200     300       0      30
 250     400       0       0
```

**原因分析**：
- 捕食压力过大 → 猎物被吃光
- 常见触发条件：
  - 捕食者太多/太强
  - 猎物 `step_energy_cost` 过高
  - 猎物繁殖太慢（`reproduce_energy_threshold` 过高）

**修复方案**：
1. 减少初始捕食者数量或增加初始猎物
2. 降低捕食的 killChance（需修改 `MyOperatorDefine.cpp`）
3. 降低猎物 `reproduce_energy_threshold`（加速繁殖）
4. 增加猎物 `max_rate`（跑得更快）

### 5. 负能量 Bug

**症状**：控制台输出 `error x y -0.xxx`

**原因**：`EnergyExchange` 中吸收量计算错误，导致环境能量被扣到负数。

**已修复**：`abs = min(abs, energy)` 夹紧确保不超出现存能量。

如果仍出现，检查其他能量交换路径是否缺少夹紧保护。

## 参数调优工作流

### 推荐调优顺序

1. **先调植物稳定性** — 确保 `Plants` 不灭绝也不爆炸
   - 用 `debug_sim` 跑 500-1000 帧
   - 观察 `P.avg` 是否稳定在 `reproduce_energy_threshold` 附近
   - 期望：植物数在 50-200 之间波动

2. **加入草食动物** — 确保 Sheep 稳定
   - 植物数应随捕食压力上下波动
   - 期望：Sheep 数在 15-80 之间，Plants ≈ 80-200

3. **加入捕食者** — 确保三条线振荡平衡
   - 经典 Lotka-Volterra 振荡模式
   - Wolf 高峰跟随 Sheep 高峰
   - 期望：Wolf 5-30, Sheep 20-60, Plants 80-200

4. **微调基因演化方向** — 观察 Sheep 基因图表
   - 如果 Wolf 灭绝，Sheep 的 escape 基因应退化
   - 如果 Wolf 持续存在，应看到 escape 和 vision 基因自然选择

### 调优记录模板

```md
## 调优记录 YYYY-MM-DD

**修改**：
- `plant_absorb_rate`: 0.2 → 0.25
- Sheep `step_energy_cost`: 0.03 → 0.05

**预期效果**：植物增长加速，Sheep 消耗增加以控制数量

**实际结果**（debug_sim 1200 帧）：
- Plants: 稳定在 80-150 ✓
- Sheep: 稳定在 25-50 ✓
- Wolves: 稳定在 5-15 ✓
- 无种群爆炸 ✓

**下一步**：可尝试加入 Fox 作为中间捕食者
```

## 典型稳定配置

### 配置 A：快速振荡（默认配置）

默认 `default_config.json` 的参数。预期产生快速 Lotka-Volterra 振荡，适合演示。

### 配置 B：缓慢稳定

调优目标：更长周期的种群波动

```json
// 修改建议
Plant step_energy_cost: 0.2 → 0.15
Plant reproduce_energy_threshold: 20 → 15
Sheep max_energy: 70 → 50
Sheep eat_intrval_max: 3 → 5
Wolf reproduce_energy_threshold: 50 → 35
```

### 配置 C：高密度

调优目标：更多生物，更密集的生态系统

```json
World length/width: 50 → 80
max_organisms_per_cell: 4 → 6
overlay_param: 1.0 → 0.5
// 降低 step_energy_cost 避免拥挤致死
```

## 基因演化分析

### 观察基因变化

在 GUI 中打开 `Gene Chart` 窗口（仅 ecosim 支持），观察 Sheep 的 6 个基因如何随时间变化：

| 基因 | 上升意味着 | 下降意味着 |
|------|-----------|-----------|
| `cohesion` | 更紧密集群 | 更分散 |
| `alignment` | 更协调一致 | 更独立 |
| `separation` | 更需要个人空间 | 更容忍拥挤 |
| `vision` | 更远视野 | 更近视 |
| `escape` | 更怕天敌 | 更无畏 |
| `food_addict` | 更积极觅食 | 更随意 |

### Wolf 存在对基因选择的影响

- **Wolf 持续存在** → `escape` 和 `vision` 应上升（怕死的羊活更久）
- **Wolf 灭绝** → `escape` 应退化（无畏不影响生存，反而是代谢负担）
- **高密度环境** → `separation` 可能下降（拥挤容忍度提高）
