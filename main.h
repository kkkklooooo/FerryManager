#ifndef FERRY_MANAGER_MAIN_H
#define FERRY_MANAGER_MAIN_H

#include <utility>
#include <vector>

extern int id;

enum OrganismType
{
    PLANT,
    PREY,
    PREDATOR
};
class Organism;
class Plant;
class Prey;
class Predator;

struct ReproduceRequest
{
    OrganismType type;
    std::pair<int, int> pos;
    int radius;
};
class World
{
    std::vector<ReproduceRequest> reproduce_requests;
    std::vector<Plant *> plants;
    std::vector<Prey *> preys;
    std::vector<Predator *> predators;

public:
    void Update();
    void Reproduce();
    void AddReproduceRequest(const ReproduceRequest &request);
    void RemoveDeadOrganisms();
    static World &GetWorld();
};

class Reproducable;

/**
 * Reproduce manager 管理生殖相关
 */
class ReproduceManager
{
public:
    
    void Update(Reproducable &reproducable);

private:
    ReproduceManager() = default;
};

/**
 * OrganismManager 管理生命体生命周期
 */
class OrganismManager
{
public:
    void Update(Organism &organism);
};

class Organism
{
public:
    float energy = 0;
    float step_energy_cost;
    float step_energy_gain;
    std::pair<int, int> Pos;
    bool active = true;
    OrganismType type;

    Organism(float step_energy_cost, float step_energy_gain, OrganismType type);
    void check_energy();
    void Step();
};

class Reproducable : public Organism
{
public:
    Reproducable(float energy_threshold, float energy_cost, int radius, float sec, float seg, OrganismType type);
    virtual ~Reproducable() = default;

    float reproduce_energy_threshold;
    float reproduce_energy_cost;
    int reproduce_radius;
    bool reproduce_able;

    virtual void Reproduce();
};

class Plant : public Reproducable
{
public:
    Plant(int id, int x, int y, int radius);

    int id;

    void Reproduce() override;
    // void check_energy() override;
    void Step();
};

#endif
