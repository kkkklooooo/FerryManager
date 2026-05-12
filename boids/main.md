● 好问题。核心矛盾是：boids 算法需要查邻居，但你不该让它依赖 World/Environment。

  ---
  架构方案：两层 API

  boids.h / boids.cpp
  ├── 底层: 纯数学函数 (无依赖)
  │   ComputeBoidsForce(self, neighbors[], w, h) → (fx, fy)
  │   └── 输入: 自己的位置/速度/基因, 邻居列表, 世界边界
  │   └── 输出: 合力向量
  │
  ├── 上层: 批量更新 (带空间网格, 自给自足)
  │   UpdateFlock(vector<BoidParticle>&, w, h)
  │   └── 内部建网格 → 查邻居 → 调底层 → 写回
  │   └── 用于独立测试, 不依赖主项目任何类
  │
  └── 主代码集成:
      Animal::BoidsMove() 调底层, 邻居来自现有 Environment::Organisms

  好处：
  - 底层纯函数可以单独单元测试（给一组坐标，验证输出向量）
  - 上层 UpdateFlock 可以写独立 exe 跑可视化（一堆彩色点绕圈）
  - 集成到主代码时，复用已有空间网格，不重复建

  ---
  具体文件

  boids.h

  #pragma once
  #include <vector>
  #include <string>
  #include <cmath>

  namespace boids {

  // ---- 基因参数 ----
  struct Genes {
      float cohesion   = 1.0f;
      float alignment  = 0.5f;
      float separation = 1.0f;
      float vision     = 3.0f;
  };

  // ---- 独立测试用的粒子 (自包含) ----
  struct Particle {
      float x, y;
      float vx, vy;
      float speed;
      int   species;    // 0=Wolf, 1=Sheep (同种聚群)
      Genes genes;
  };

  // ---- 底层: 邻居查询用的轻量引用 ----
  struct Neighbor {
      float x, y;
      float vx, vy;
      int   species;
  };

  // 底层 API: 计算单个粒子的受力
  // neighbors 由调用方提供(空间网格 或 暴力遍历)
  void ComputeForce(
      float selfX, float selfY,
      float selfVx, float selfVy,
      float speed, int species, const Genes& g,
      const std::vector<Neighbor>& neighbors,
      int worldW, int worldH,
      float& outFx, float& outFy
  );

  // 上层 API: 批量更新一群粒子(自带空间网格)
  void UpdateFlock(std::vector<Particle>& flock, int w, int h);

  } // namespace boids

  boids.cpp 核心逻辑

  void boids::ComputeForce(/*...*/) {
      float cx = 0, cy = 0;  // cohesion
      float ax = 0, ay = 0;  // alignment
      float sx = 0, sy = 0;  // separation
      int count = 0;

      for (auto& n : neighbors) {
          if (n.species != species) continue;  // 只跟同种聚群
          float dx = n.x - selfX;
          float dy = n.y - selfY;
          float dist = std::sqrt(dx*dx + dy*dy);
          if (dist < 0.01f || dist > g.vision) continue;

          count++;
          cx += dx; cy += dy;                    // cohesion
          ax += n.vx; ay += n.vy;               // alignment
          sx -= dx / (dist*dist); sy -= dy / (dist*dist);  // separation
      }

      if (count == 0) { outFx = outFy = 0; return; }

      cx = (cx / count) * g.cohesion;
      cy = (cy / count) * g.cohesion;
      ax = (ax / count) * g.alignment;
      ay = (ay / count) * g.alignment;
      sx *= g.separation;
      sy *= g.separation;

      // 噪声
      float nx = ((float)rand()/RAND_MAX - 0.5f) * 0.3f;
      float ny = ((float)rand()/RAND_MAX - 0.5f) * 0.3f;

      outFx = cx + ax + sx + nx;
      outFy = cy + ay + sy + ny;
  }

  test_boids.cpp — 独立测试程序

  CMake 新 target: boids_test
  链接: boids.cpp + imgui (只需渲染)
  不依赖: Organism / World / Environment

  功能:
  - 初始化 50 个粒子, 随机位置/速度
  - 每帧调 UpdateFlock()
  - ImGui 画布上画彩色圆点
  - Slider 调 cohesion/alignment/separation/vision
  - 两个 species, 不同颜色, 看聚群效果

  ---
  集成到主代码 (未来)

  Animal::BoidsMove() 只需调底层:

  void Animal::BoidsMove() {
      std::vector<boids::Neighbor> neighbors;
      // 从 Environment::Organisms 收集邻居 (复用已有网格)
      int vr = genes.vision;
      for (int dy = -vr; dy <= vr; dy++)
      for (int dx = -vr; dx <= vr; dx++) {
          int ny = Pos.second + dy, nx = Pos.first + dx;
          if (ny<0||ny>=WorldH||nx<0||nx>=WorldW) continue;
          for (auto* org : envs[ny*W+nx]->Organisms) {
              if (org == this) continue;
              neighbors.push_back({(float)org->Pos.first, (float)org->Pos.second,
                                   static_cast<Animal*>(org)->vx,
                                   static_cast<Animal*>(org)->vy,
                                   org->name == "Wolf" ? 0 : 1});
          }
      }

      float fx, fy;
      boids::ComputeForce(Pos.first, Pos.second, vx, vy, rate,
                          (name == "Wolf" ? 0 : 1), genes,
                          neighbors, WorldW, WorldH, fx, fy);
      // 应用力到速度/位置...
  }

  ---
  CMake 结构

  # 主程序
  add_executable(ecosim
      test_main.cpp OrganismDefine.cpp ...
      boids.cpp          # <-- 共享
  )

  # 独立 boids 测试
  add_executable(boids_test
      test_boids.cpp
      boids.cpp
      imgui/...          # 只需渲染
  )

  ---
  总结

  ┌────────┬─────────────────────┬─────────────────────────┬───────────────────────┐
  │   层   │        函数         │          依赖           │         用途          │
  ├────────┼─────────────────────┼─────────────────────────┼───────────────────────┤
  │ 底层   │ ComputeForce()      │ 无(纯数学)              │ 主代码集成 + 单元测试 │
  ├────────┼─────────────────────┼─────────────────────────┼───────────────────────┤
  │ 上层   │ UpdateFlock()       │ ComputeForce + 自建网格 │ 独立可视化测试        │
  ├────────┼─────────────────────┼─────────────────────────┼───────────────────────┤
  │ 主代码 │ Animal::BoidsMove() │ ComputeForce + 现有网格 │ 生产环境              │
  └────────┴─────────────────────┴─────────────────────────┴───────────────────────┘

  要我开始写 boids.h / boids.cpp / test_boids.cpp 吗？