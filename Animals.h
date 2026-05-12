#pragma once
#include"Organism.h"
#include"MyOperator.h"
#include"Config.h"
#include"World.h"
// 这里是标签二: 工厂

class UserAnimal :public Animal {
public:
	UserAnimal(int iD,int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, AnimalConfig org)
		:Animal(iD, x, y, radius, reproduce_energy_threshold, reproduce_energy_cost, org)
	{}
	static AnimalConfig FindAnimalConfig(const std::string& name) {
		auto& animals = TestConfig::GetTestConfig().The_Animals;
		for (auto& a : animals) {
			if (a.name == name) return a;
		}
		return TestConfig::GetTestConfig().Default_Animal_Config;
	}
};

//系统的原有用注册表
//此时的clonfg都没有创建只能硬编码？
static AnimalRegistrator wolf("Wolf", [](int id, int x, int y, int radius) {return new UserAnimal(id, x, y,radius, 25, World::GetWorld().conf.Organism_reproduce_energy_cost,UserAnimal::FindAnimalConfig("Wolf")); });

static AnimalRegistrator sheep("Sheep", [](int id, int x, int y, int radius) {return new UserAnimal(id, x, y, radius, 25, World::GetWorld().conf.Organism_reproduce_energy_cost, UserAnimal::FindAnimalConfig("Sheep")); });