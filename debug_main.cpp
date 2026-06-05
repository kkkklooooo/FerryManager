#include "Config.h"
#include "World.h"
#include "Animals.h"
#include <cstdio>
#include <fstream>
#include <cmath>

int main() {
    TestConfig tc;
    {
        const char* paths[] = {
            "default_config.json",
            "../default_config.json",
            "config/default_config.json",
            "../config/default_config.json"
        };
        std::ifstream f;
        for (auto p : paths) {
            f.open(p);
            if (f.is_open()) break;
        }
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

    printf("\n%5s %6s %6s %6s %7s %7s %7s %7s %7s %7s %7s %7s %7s %6s %6s %6s\n",
        "Step", "Plants", "Sheep", "Wolves",
        "P.min", "P.avg", "P.max",
        "S.min", "S.avg", "S.max",
        "W.min", "W.avg", "W.max",
        "P_req", "S_req", "W_req");
    printf("----- ------ ------ ------ ------- ------- ------- ------- ------- ------- ------- ------- ------- ------ ------ ------\n");

    int totalSteps = 1200;
    int maxOrgs = 10000; // bail if explosion
    for (int step = 1; step <= totalSteps; step++) {
        const auto& reqsBefore = w.GetReproduceRequests();
        size_t reqCountBefore = reqsBefore.size();

        w.Update();

        const auto& orgs = w.GetReproducas();
        int plants = 0, sheep = 0, wolves = 0;
        float plantE = 0, pmin = 1e9f, pmax = -1e9f;
        float sheepE = 0, smin = 1e9f, smax = -1e9f;
        float wolfE = 0, wmin = 1e9f, wmax = -1e9f;
        for (auto* o : orgs) {
            if (!o || !o->active) continue;
            if (o->type == PLANT) {
                plants++;
                plantE += o->energy;
                if (o->energy < pmin) pmin = o->energy;
                if (o->energy > pmax) pmax = o->energy;
            } else if (o->name == "Sheep") {
                sheep++;
                sheepE += o->energy;
                if (o->energy < smin) smin = o->energy;
                if (o->energy > smax) smax = o->energy;
            } else if (o->name == "Wolf") {
                wolves++;
                wolfE += o->energy;
                if (o->energy < wmin) wmin = o->energy;
                if (o->energy > wmax) wmax = o->energy;
            }
        }
        if (plants == 0) { pmin = pmax = 0; }
        if (sheep == 0)  { smin = smax = 0; }
        if (wolves == 0) { wmin = wmax = 0; }

        int totalOrgs = plants + sheep + wolves;
        if (totalOrgs > maxOrgs) {
            printf("*** POPULATION EXPLOSION at step %d: total=%d plants=%d sheep=%d ***\n",
                step, totalOrgs, plants, sheep);
            break;
        }

        // count reproduce requests this step
        const auto& reqs = w.GetReproduceRequests();
        int plantReqs = 0, sheepReqs = 0, wolfReqs = 0;
        for (auto& r : reqs) {
            if (r.name == "Gress") plantReqs++;
            if (r.name == "Sheep") sheepReqs++;
            if (r.name == "Wolf") wolfReqs++;
        }

        bool detail = (step <= 30) || (step % 100 == 0) || (step % 100 == 99) || (wolves == 0) || (sheep == 0);
        if (detail)
            printf("%5d %6d %6d %6d %7.1f %7.1f %7.1f %7.1f %7.1f %7.1f %7.1f %7.1f %7.1f %6d %6d %6d\n",
                step, plants, sheep, wolves,
                pmin, plantE/std::max(1,plants), pmax,
                smin, sheepE/std::max(1,sheep), smax,
                wmin, wolfE/std::max(1,wolves), wmax,
                plantReqs, sheepReqs, wolfReqs);
    }

    return 0;
}
