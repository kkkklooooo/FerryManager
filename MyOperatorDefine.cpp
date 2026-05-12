#include"MyOperator.h"
#include<cassert>
#include<cstdio>
#include<algorithm>
using Creator = std::function<Animal*( int id, int x, int y, int radius)>;
void MyOperator::operator()(Reproducable* a, Reproducable* b) {
    bool aEb = false;
    bool bEa = false;
    //TODO 给环境加上吃死了的动物的能量的方法
    if (find(a->diet.begin(), a->diet.end(), b->name) != a->diet.end()) {
        aEb = true;
    }
    if (find(b->diet.begin(), b->diet.end(), a->name) != b->diet.end()) {
        bEa = true;
    }
    if (aEb && !bEa) {
        a->energy += b->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        b->energy -= b->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        b->active = false;
        dynamic_cast<Animal*>(a)->OnEatInterval();
        // assert(a->energy >= -100&& b->energy >= -100);
        // printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
        return;
    }
    if (bEa && !aEb) {
        b->energy += a->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        a->energy -= a->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        a->active = false;
        dynamic_cast<Animal*>(b)->OnEatInterval();
        // assert(a->energy >= -100&& b->energy >= -100);
        // printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
        return;
    }
    if (aEb && bEa) {
        if (a->energy >= b->energy) {
            b->active = false;
            printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
            return;
        }
        else {
            a->active = false;
            printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
            return;
        }
    }
}


void MyOperator::register_Animal_Create(std::string  name, Creator creator) {
    registry()[name] = std::move(creator);
}


Reproducable* MyOperator::operator()(int x,int y,int r, std::string  n,int id) {
    auto it = registry().find(n);
    if (it != registry().end()) {// 调用注册的创建函数
        return it->second( id, x, y,r); 
    }
    return nullptr;

} 
Reproducable* MyOperator::operator()(ReproduceRequest& x,int id) {  
    auto it = registry().find(x.name);
    if (it != registry().end()) {// 调用注册的创建函数
        return it->second( id, x.pos.first, x.pos.second, x.radius); 
    }
    return nullptr;
}

std::unordered_map <std::string, Creator>& MyOperator::registry() {
    static std::unordered_map<std::string, Creator> reg;
    return reg;
}



MyOperator& MyOperator::GetOp() {
    static MyOperator Instance;
    return Instance;
}