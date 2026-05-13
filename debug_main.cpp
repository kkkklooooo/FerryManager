#include "Config.h"
#include "World.h"
#include "Animals.h"
#include <cstdio>
#include <fstream>
#include <cmath>

int main() {
    TestConfig tc;
    {
        std::ifstream f("config/default_config.json");
        if (f.is_open()) {
            json j; f >> j;
            tc = j.get<TestConfig>();
            printf("Loaded default_config.json\n");
        } else {
            printf("No config file, using defaults\n");
        }
    }
    InitGameConfig(tc);
    World& w = World::GetWorld(tc);
    w.CurrentWeather = SUN;

    int ww = w.GetWidth(), wh = w.GetHeight();
    printf("World: %dx%d  Plants: %d  Animals: %d  Envs: %d\n",
        ww, wh,
        (int)w.GetReproducas().size(),
        (int)std::count_if(w.GetReproducas().begin(), w.GetReproducas().end(),
            [](auto* o){ return o && o->type == ANIMAL; }),
        (int)w.GetEnvironments().size());

    printf("\n%6s %7s %7s %7s %7s %8s %8s %8s %8s\n",
        "Step", "Plants", "Sheep", "Wolves", "MaxSpd", "P.min", "P.avg", "P.max", "EnvE");
    printf("------ ------- ------- ------- ------- -------- -------- -------- --------\n");

    int totalSteps = 500;
    for (int step = 1; step <= totalSteps; step++) {
        w.Update();

        const auto& orgs = w.GetReproducas();
        int plants = 0, sheep = 0, wolves = 0;
        float plantE = 0, pmin = 1e9, pmax = -1;
        float maxSpeed = 0;
        for (auto* o : orgs) {
            if (!o || !o->active) continue;
            if (o->type == PLANT) {
                plants++;
                plantE += o->energy;
                if (o->energy < pmin) pmin = o->energy;
                if (o->energy > pmax) pmax = o->energy;
            } else {
                if (o->name == "Sheep") sheep++;
                if (o->name == "Wolf")  wolves++;
                float spd = std::sqrt(o->velocity.first*o->velocity.first + o->velocity.second*o->velocity.second);
                if (spd > maxSpeed) maxSpeed = spd;
            }
        }
        if (plants == 0) { pmin = pmax = 0; }

        float envE = 0;
        for (auto* e : w.GetEnvironments()) envE += e->energy;

        if (step <= 15 || step % 20 == 0 || step % 20 == 19)
            printf("%6d %7d %7d %7d %7.1f %8.1f %8.1f %8.1f %8.1f\n",
                step, plants, sheep, wolves, maxSpeed,
                pmin, plantE/std::max(1,plants), pmax, envE);
    }

    return 0;
}
