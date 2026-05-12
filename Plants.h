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
		auto& plants = World::GetWorld().game_conf.The_Plants;
		for (auto& p : plants) {
			if (p.name == name) return p;
		}
		return World::GetWorld().game_conf.Default_Plant_Config;
	}
};
