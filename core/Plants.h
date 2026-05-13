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
		auto& plants = TestConfig::GetTestConfig().The_Plants;
		for (auto& a : plants) {
			if (a.name == name) return TestConfig::GetTestConfig().Check_Plant(a);
		}
		printf("\033[不知名的植物\033[0m\n");
		return TestConfig::GetTestConfig().Default_Plant_Config;
	}
};

static PlantRegistrator Gress("Gress", [](int id, int x, int y, int radius) {return new UserPlant(id, x, y, radius, Config::GetConfig().Organism_reproduce_energy_threshold, Config::GetConfig().Organism_reproduce_energy_cost, UserPlant::FindPlantConfig("Gress")); });