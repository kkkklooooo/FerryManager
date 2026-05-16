#include "boids.h"
#include "Genes.h"
#include <algorithm>
#include <vector>

namespace boids
{
    Particle CreateParticle(Reproducable *a, Genes gs)
    {
        std::vector<int> diet;
        std::vector<int> predator;
        std::transform(a->diet.begin(), a->diet.end(), std::back_inserter(diet), [](const std::string &s)
                       { return s == "Wolf" ? 0 : 1; });
        std::transform(a->predator.begin(), a->predator.end(), std::back_inserter(predator), [](const std::string &s)
                       { return s == "Wolf" ? 0 : 1; });
        return {
            (float)a->explicit_pos.first,
            (float)a->explicit_pos.second,
            a->velocity.first,
            a->velocity.second,
            0.0f,
            (a->name == "Wolf") ? 0 : 1,
            gs,
            diet,
            predator};
    }

    void FillNeighbors(std::pair<int, int> pos, float vision, std::vector<Particle> &neighbors, World &world)
    {
        neighbors.clear();
        int w = world.GetWidth(), h = world.GetHeight();
        int rad = (int)vision;
        for (int dy = -rad; dy <= rad; dy++)
            for (int dx = -rad; dx <= rad; dx++)
            {
                int x = pos.first + dx;
                int y = pos.second + dy;
                if (x < 0 || x >= w || y < 0 || y >= h)
                    continue;
                auto &os = world.GetEnvironments()[y * w + x]->Organisms;
                for (auto *org : os)
                {
                    if (org && org->type == ANIMAL)
                        neighbors.push_back(CreateParticle(org, static_cast<Animal *>(org)->genes));
                }
            }
    }

    std::pair<float, float> ComputeFinalForce(Particle self, std::vector<Particle> &neighbors)
    {
        float cx = 0, cy = 0;
        float ax = 0, ay = 0;
        float sx = 0, sy = 0;

        float food_x = 0, food_y = 0;
        float escape_x = 0, escape_y = 0;

        int count_food = 0;
        int count = 0;

        for (auto &i : neighbors)
        {
            float dx = i.x - self.x;
            float dy = i.y - self.y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance > self.genes.vision || distance < 0.001f)
                continue;
            if (i.species != self.species)
            {
                if (self.diet.size() > 0 && std::find(self.diet.begin(), self.diet.end(), i.species) != self.diet.end())
                {
                    food_x += i.x;
                    food_y += i.y;
                    count_food++;
                }
                if (self.predator.size() > 0 && std::find(self.predator.begin(), self.predator.end(), i.species) != self.predator.end())
                {
                    escape_x -= dx / (distance * distance);
                    escape_y -= dy / (distance * distance);
                }
            }
            cx += i.x;
            cy += i.y;
            ax += i.vx;
            ay += i.vy;
            count++;
            sx -= dx / (distance * distance);
            sy -= dy / (distance * distance);
        }

        cx = cx / count;
        cy = cy / count;
        ax = ax / count;
        ay = ay / count;
        if (count_food > 0)
        {

            food_x = food_x / count_food;
            food_y = food_y / count_food;
        }
        else
        {
            food_x = self.x;
            food_y = self.y;
        }

        float nx = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        float ny = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        if (count <= 0)
            return std::make_pair(nx, ny);
        return std::make_pair(
            self.genes.cohesion * (cx - self.x) + self.genes.alignment * (ax - self.vx) + self.genes.separation * (sx) + nx + self.genes.food_addict * (food_x - self.x) + self.genes.escape * (escape_x),
            self.genes.cohesion * (cy - self.y) + self.genes.alignment * (ay - self.vy) + self.genes.separation * (sy) + ny + self.genes.escape * (escape_y) + self.genes.food_addict * (food_y - self.y));
    }
}
