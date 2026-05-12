#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>

namespace boids
{
    struct Genes
    {
        float cohesion = 1.0f;
        float alignment = 0.5f;
        float separation = 1.0f;
        float vision = 3.0f;
    };
    // ---- 独立测试用的粒子 (自包含) ----
    struct Particle
    {
        float x, y;
        float vx, vy;
        float speed;
        int species; // 0=Wolf, 1=Sheep (同种聚群)
        Genes genes;
    };
    std::pair<float,float>ComputeFinalForce(float self_x, float self_y, float self_vx, float self_vy,std::vector<Particle> &neighbors){
        
    }
}