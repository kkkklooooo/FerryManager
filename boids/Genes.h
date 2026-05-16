#pragma once
#include<vector>

namespace boids
{
    struct Genes
    {
        float cohesion = 1.0f;
        float alignment = 0.5f;
        float separation = 1.0f;
        float vision = 6.0f;
        float escape = 0.5f;
        float food_addict = 0.5f;
    };
    struct Particle
    {
        float x, y;
        float vx, vy;
        float speed;
        int species; // 0=Wolf, 1=Sheep
        Genes genes;
        std::vector<int> diet;
        std::vector<int> predator;
    };
}
