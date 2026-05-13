#pragma once
#include"World.h"
#include"Animals.h"
#include"Plants.h"
#include<functional>
#include<map>
class MyOperator {
public:
	using Creator = std::function<UserAnimal*( int id, int x, int y, int radius)>;
	using PlantCreator = std::function<UserPlant* (int id, int x, int y, int radius)>; //注册官方植物的

	//ע�ắ��
	static void register_Animal_Create(std::string name, Creator creator);
	static void register_Plant_Create(std::string name, PlantCreator creator);
	void operator()(Reproducable* a,Reproducable* b);
	Reproducable* operator()(ReproduceRequest &x,int id);
	Reproducable* operator()(int x,int y,int r, std::string n,int id);
	static MyOperator& GetOp();
private:
	//ȫ��Ψһע���
	static std::unordered_map <std::string , Creator>& registry();
	static std::unordered_map <std::string, PlantCreator>& Plantregistry();
};

struct PlantRegistrator {
	PlantRegistrator(std::string name, MyOperator::PlantCreator creator) {
		MyOperator::register_Plant_Create(name, std::move(creator));
	}
	PlantRegistrator() = default;
};

static PlantRegistrator Gress("Gress", [](int id, int x, int y, int radius) {return new UserPlant(id, x, y, radius, Config::GetConfig().Organism_reproduce_energy_threshold, Config::GetConfig().Organism_reproduce_energy_cost, UserPlant::FindPlantConfig("Gress")); });

struct AnimalRegistrator {
	AnimalRegistrator(std::string name, MyOperator::Creator creator) {
		MyOperator::register_Animal_Create(name, std::move(creator));
	}
	AnimalRegistrator() = default;
};

static AnimalRegistrator wolf("Wolf", [](int id, int x, int y, int radius)->UserAnimal* {return new UserAnimal(id, x, y, radius, Config::GetConfig().Organism_reproduce_energy_threshold, Config::GetConfig().Organism_reproduce_energy_cost, UserAnimal::FindAnimalConfig("Wolf")); });

static AnimalRegistrator sheep("Sheep", [](int id, int x, int y, int radius)->UserAnimal* {return new UserAnimal(id, x, y, radius, Config::GetConfig().Organism_reproduce_energy_threshold, Config::GetConfig().Organism_reproduce_energy_cost, UserAnimal::FindAnimalConfig("Sheep")); });