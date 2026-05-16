#include "MyOperator.h"
#include "World.h"
#include <cassert>
#include <cstdio>
#include <algorithm>
#include<random>
using Creator = std::function<UserAnimal *(int id, int x, int y, int radius,std::optional<boids::Genes> g)>;
using PlantCreator = std::function<UserPlant *(int id, int x, int y, int radius ,    std::optional<boids::Genes> g)>; // 注册官方植物的
extern double rand01(); 

void MyOperator::operator()(Reproducable *a, Reproducable *b)
{
    bool aEb = false;
    bool bEa = false;
    // TODO ���������ϳ����˵Ķ���������ķ���
    if (find(a->diet.begin(), a->diet.end(), b->name) != a->diet.end())
    {
        aEb = true;
    }
    if (find(b->diet.begin(), b->diet.end(), a->name) != b->diet.end())
    {
        bEa = true;
    }
    if (aEb && !bEa)
    {
        Animal *aAnimal = dynamic_cast<Animal *>(a);
        if (aAnimal->eat_intrval > 0)
            return;
        float hunger = aAnimal->max_energy - a->energy;
        if (hunger <= 0) return;
        float absorb = World::GetWorld().conf.Organism_animal_absorb_rate;
        float loss  = World::GetWorld().conf.Organism_loss_rate;
        float bite  = std::min({hunger / loss, b->energy * absorb, b->energy});
        if (bite <= 0) return;
        a->energy += bite * loss;
        if (a->energy > aAnimal->max_energy) a->energy = (float)aAnimal->max_energy;
        b->energy -= bite;
        aAnimal->OnEatInterval();
        return;
    }
    if (bEa && !aEb)
    {
        Animal *bAnimal = dynamic_cast<Animal *>(b);
        if (bAnimal->eat_intrval > 0)
            return;
        float hunger = bAnimal->max_energy - b->energy;
        if (hunger <= 0) return;
        float absorb = World::GetWorld().conf.Organism_animal_absorb_rate;
        float loss  = World::GetWorld().conf.Organism_loss_rate;
        float bite  = std::min({hunger / loss, a->energy * absorb, a->energy});
        if (bite <= 0) return;
        b->energy += bite * loss;
        if (b->energy > bAnimal->max_energy) b->energy = (float)bAnimal->max_energy;
        a->energy -= bite;
        bAnimal->OnEatInterval();
        return;
    }
    if (aEb && bEa)
    {
        Animal *aAnimal = dynamic_cast<Animal *>(a);
        Animal *bAnimal = dynamic_cast<Animal *>(b);
        if (aAnimal && bAnimal) {
            float totalE = a->energy + b->energy;
            if (totalE <= 0) return;
            float aRatio = a->energy / totalE;
            float absorb = World::GetWorld().conf.Organism_animal_absorb_rate;
            float aDmg = b->energy * absorb * (1.0f - aRatio);
            float bDmg = a->energy * absorb * aRatio;
            if (aAnimal->eat_intrval <= 0 && aAnimal->energy < aAnimal->max_energy) {
                float hunger = aAnimal->max_energy - a->energy;
                float gain = std::min(bDmg * World::GetWorld().conf.Organism_loss_rate, hunger);
                a->energy += gain;
                if (a->energy > aAnimal->max_energy) a->energy = (float)aAnimal->max_energy;
                aAnimal->OnEatInterval();
            }
            b->energy -= bDmg;
            if (bAnimal->eat_intrval <= 0 && bAnimal->energy < bAnimal->max_energy) {
                float hunger = bAnimal->max_energy - b->energy;
                float gain = std::min(aDmg * World::GetWorld().conf.Organism_loss_rate, hunger);
                b->energy += gain;
                if (b->energy > bAnimal->max_energy) b->energy = (float)bAnimal->max_energy;
                bAnimal->OnEatInterval();
            }
            a->energy -= aDmg;
        } else {
            if (a->energy >= b->energy) b->active = false;
            else                       a->active = false;
        }
        return;
    }
}

void MyOperator::register_Animal_Create(std::string name, Creator creator)
{
    registry()[name] = std::move(creator);
}

void MyOperator::register_Plant_Create(std::string name, PlantCreator creator)
{
    Plantregistry()[name] = std::move(creator);
}

Reproducable *MyOperator::operator()(int x, int y, int r, std::string n, int id,std::optional<boids::Genes> genes)
{
    auto it_1 = registry().find(n);
    if (it_1 != registry().end())
    { // ����ע��Ĵ�������

        return it_1->second(id, x, y, r, genes);
    }
    auto it_2 = Plantregistry().find(n);
    if (it_2 != Plantregistry().end())
    { // ����ע��Ĵ�������
        return it_2->second(id, x, y, r,std::nullopt);
    }
    printf("77 %s\n", n.data());
    return nullptr;
}
Reproducable *MyOperator::operator()(ReproduceRequest &x, int id)
{
    auto it = registry().find(x.name);
    if (it != registry().end())
    { // ����ע��Ĵ�������
        return it->second(id, x.pos.first, x.pos.second, x.radius, x.new_genes);
    }
    auto it_2 = Plantregistry().find(x.name);
    if (it_2 != Plantregistry().end())
    { // ����ע��Ĵ�������
        return it_2->second(id, x.pos.first, x.pos.second, x.radius,std::nullopt);
    }
    return nullptr;
    return nullptr;
}

std::unordered_map<std::string, Creator> &MyOperator::registry()
{
    static std::unordered_map<std::string, Creator> reg;
    return reg;
}

std::unordered_map<std::string, PlantCreator> &MyOperator::Plantregistry()
{
    static std::unordered_map<std::string, PlantCreator> reg;
    return reg;
}

MyOperator &MyOperator::GetOp()
{
    static MyOperator Instance;
    return Instance;
}