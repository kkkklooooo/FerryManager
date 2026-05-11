#include"MyOperator.h"
#include"Word.h"
#include<cassert>
#include<cstdio>
#include<algorithm>

// ====== MyOperator 用虚函数 canEat 做捕食判断，与 diet 解耦 ======
using Creator = std::function<Animal*(int id, int x, int y, int radius)>;

void MyOperator::operator()(Reproducable* a, Reproducable* b) {
    bool aEb = a->canEat(b);
    bool bEa = b->canEat(a);

    if (aEb && !bEa) {
        a->energy += b->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        b->energy -= b->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        b->active = false;
        printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
    } else if (bEa && !aEb) {
        b->energy += a->energy * World::GetWorld().conf.Organism_animal_absorb_rate * World::GetWorld().conf.Organism_loss_rate;
        a->energy -= a->energy * World::GetWorld().conf.Organism_animal_absorb_rate;
        a->active = false;
        printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
    } else if (aEb && bEa) {
        if (a->energy >= b->energy) {
            b->active = false;
            printf("\033[31m%s eat %s\033[0m\n", a->name, b->name);
        } else {
            a->active = false;
            printf("\033[31m%s eat %s\033[0m\n", b->name, a->name);
        }
    }
}


void MyOperator::register_Animal_Create(OrganismName name, Creator creator) {
    registry()[name] = std::move(creator);
}


Reproducable* MyOperator::operator()(ReproduceRequest& x,int id) {  
    auto it = registry().find(x.name);
    if (it != registry().end()) {// ����ע��Ĵ�������
        return it->second( id, x.pos.first, x.pos.second, x.radius); 
    }
    return nullptr;
}

std::unordered_map <OrganismName, Creator>& MyOperator::registry() {
    static std::unordered_map<OrganismName, Creator> reg;
    return reg;
}



MyOperator& MyOperator::GetOp() {
    static MyOperator Instance;
    return Instance;
}