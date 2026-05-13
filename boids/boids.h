#pragma once
#include "World.h"
#include "Organism.h"
#include "Genes.h"
#include <cmath>
#include <vector>
#include <utility>
#include <cstdlib>

namespace boids
{
    

    Particle CreateParticle(Reproducable* a, Genes gs);
    void FillNeighbors(std::pair<int, int> pos, float vision, std::vector<Particle>& neighbors, World& world);
    std::pair<float, float> ComputeFinalForce(Particle self, std::vector<Particle>& neighbors);
}