#include "boids.h"
#include"Genes.h"

namespace boids
{
    Particle CreateParticle(Reproducable* a, Genes gs)
    {
        return {
            (float)a->explicit_pos.first,
            (float)a->explicit_pos.second,
            a->velocity.first,
            a->velocity.second,
            0.0f,
            (a->name == "Wolf") ? 0 : 1,
            gs
        };
    }

    void FillNeighbors(std::pair<int, int> pos, float vision, std::vector<Particle>& neighbors, World& world)
    {
        neighbors.clear();
        int w = world.GetWidth(), h = world.GetHeight();
        int rad = (int)vision;
        for (int dy = -rad; dy <= rad; dy++)
            for (int dx = -rad; dx <= rad; dx++) {
                int x = pos.first + dx;
                int y = pos.second + dy;
                if (x < 0 || x >= w || y < 0 || y >= h) continue;
                auto& os = world.GetEnvironments()[y * w + x]->Organisms;
                for (auto* org : os) {
                    if (org && org->type == ANIMAL)
                        neighbors.push_back(CreateParticle(org, static_cast<Animal*>(org)->genes));
                }
            }
    }

    std::pair<float, float> ComputeFinalForce(Particle self, std::vector<Particle>& neighbors)
    {
        float cx = 0, cy = 0;
        float ax = 0, ay = 0;
        float sx = 0, sy = 0;
        int count = 0;

        for (auto& i : neighbors)
        {
            if (i.species != self.species)
                continue;
            float dx = i.x - self.x;
            float dy = i.y - self.y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance > self.genes.vision || distance < 0.001f)
                continue;
            cx += i.x;
            cy += i.y;
            ax += i.vx;
            ay += i.vy;
            count++;
            sx -= dx / (distance * distance);
            sy -= dy / (distance * distance);
        }
        if (count <= 0)
            return std::make_pair(0.0f, 0.0f);
        cx = cx / count;
        cy = cy / count;
        ax = ax / count;
        ay = ay / count;
        float nx = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        float ny = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;

        return std::make_pair(
            self.genes.cohesion * (cx - self.x) + self.genes.alignment * (ax - self.vx) + self.genes.separation * (sx) + nx,
            self.genes.cohesion * (cy - self.y) + self.genes.alignment * (ay - self.vy) + self.genes.separation * (sy) + ny);
    }
}
