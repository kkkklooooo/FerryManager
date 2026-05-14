#include "MyOperator.h"
#include "World.h"
#include <cassert>
#include <cstdio>
#include <algorithm>
using Creator = std::function<UserAnimal *(int id, int x, int y, int radius,std::optional<boids::Genes> g)>;
using PlantCreator = std::function<UserPlant *(int id, int x, int y, int radius ,    std::optional<boids::Genes> g)>; // 注册官方植物的


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
            return; // ����eat,��������һ��eat
        a->energy += b->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        b->energy -= b->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        b->active = false;
        aAnimal->OnEatInterval();
        // assert(a->energy >= -100&& b->energy >= -100);
        // printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
        return;
    }
    if (bEa && !aEb)
    {
        Animal *bAnimal = dynamic_cast<Animal *>(b);
        if (bAnimal->eat_intrval > 0)
            return;
        b->energy += a->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        a->energy -= a->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        a->active = false;
        bAnimal->OnEatInterval();
        // assert(a->energy >= -100&& b->energy >= -100);
        // printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
        return;
    }
    if (aEb && bEa)
    {
        if (a->energy >= b->energy)
        {
            b->active = false;
            printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
            return;
        }
        else
        {
            a->active = false;
            printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
            return;
        }
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