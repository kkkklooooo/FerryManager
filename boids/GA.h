#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include <cstdio>
#include "Genes.h"

namespace GA{
    boids::Genes Fusion(const boids::Genes &a, const boids::Genes &b){
        std::normal_distribution<float> noise(0.0f, 0.15f);  // 均值0，标准差0.15
        std::random_device rd;
        std::mt19937 gen(rd());
        auto blend=[&](float a,float b){
            float r= (a+b)*0.5f;
            return r+noise(gen);
        };
        return boids::Genes{blend(a.alignment,b.alignment),
                            blend(a.cohesion,b.cohesion),
                            blend(a.separation,b.separation),
                            blend(a.vision,b.vision)};
    } 
    // void Mutation(boids::Genes& new_gene)
    // {
    //     std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    //     std::mt19937 gen;
    //     float value = dist(gen);
    // }
}
