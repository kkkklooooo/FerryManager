
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
    std::pair<float, float> ComputeFinalForce(Particle self, std::vector<Particle> &neighbors)
    {
        float cx = 0, cy = 0; // cohesion
        float ax = 0, ay = 0; // alignment
        float sx = 0, sy = 0; // separation
        int count = 0;

        for (auto &i : neighbors)
        {
            if (i.species != self.species)
                continue;
            float dx = i.x - self.x;
            float dy = i.y - self.y;
            float distance = sqrt(dx * dx + dy * dy);
            if(distance > self.genes.vision || distance < 0.001f) continue;
            cx += i.x;
            cy += i.y;
            ax += i.vx;
            ay += i.vy;
            count++;
            sx -= dx / (distance*distance);
            sy -= dy / (distance*distance);
        }
        if(count<=0) return std::make_pair(0,0);
        cx = cx / count;
        cy = cy / count;
        ax = ax / count;
        ay = ay / count;
        float nx = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        float ny = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;

        
        return std::make_pair(
            self.genes.cohesion * (cx-self.x) + self.genes.alignment * (ax-self.vx) + self.genes.separation * (sx) + nx,
            self.genes.cohesion * (cy-self.y) + self.genes.alignment * (ay-self.vy) + self.genes.separation * (sy) + ny);
    }
}