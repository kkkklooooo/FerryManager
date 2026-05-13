#pragma once
#include"Organism.h"
#include"config/Config.h"
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
			if (a.name == name) return TestConfig::GetTestConfig().Check_Animal(a);
		}
		return TestConfig::GetTestConfig().Default_Animal_Config;
	}
};


