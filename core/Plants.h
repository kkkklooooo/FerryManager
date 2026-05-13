#pragma once
#include"Organism.h"
#include"config/Config.h"

class UserPlant :public Plant {
public:
	UserPlant(int iD, int x, int y, int radius, float reproduce_energy_threshold, float reproduce_energy_cost, PlantConfig org)
		:Plant(iD, x, y, radius, reproduce_energy_threshold, reproduce_energy_cost, org)
	{}
	static PlantConfig FindPlantConfig(const std::string& name) {
		auto& plants = TestConfig::GetTestConfig().The_Plants;
		for (auto& a : plants) {
			
			if (a.name == name) return TestConfig::GetTestConfig().Check_Plant(a);
			else
			{
				printf("%s\n", a.name.data());
				printf_s("lll %s\n", name.data());
			}
		}
		return TestConfig::GetTestConfig().Default_Plant_Config;
	}
};

