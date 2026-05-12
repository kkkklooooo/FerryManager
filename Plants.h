#pragma once
#include"Organism.h"
#include"MyOperator.h"
#include"Config.h"

class UserPlant :public Plant {
public:
	UserPlant(int iD, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, PlantConfig org)
		:Plant(iD, x, y, radius, reproduce_energy_threshold, reproduce_energy_cost, org)
	{}
	static PlantConfig FindPlantConfig(const std::string& name) {
		auto& animals = TestConfig::GetTestConfig().The_Plants;
		for (auto& a : animals) {
			if (a.name == name) return a;
		}
		printf("\033[返回了不知道的东西\033[0m\n");
		return TestConfig::GetTestConfig().Default_Plant_Config;
	}
};
