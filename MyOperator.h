#pragma once
#include "Organism.h"
#include <functional>
#include <unordered_map>

class MyOperator {
public:
    using Creator = std::function<Animal*(int id, int x, int y, int radius)>;

    static void register_Animal_Create(OrganismName name, Creator creator);

    void operator()(Reproducable* a, Reproducable* b);
    Reproducable* operator()(ReproduceRequest& x, int id);
    static MyOperator& GetOp();

private:
    static std::unordered_map<OrganismName, Creator>& registry();
};

struct AnimalRegistrator {
    AnimalRegistrator(OrganismName name, MyOperator::Creator creator) {
        MyOperator::register_Animal_Create(name, std::move(creator));
    }
};
