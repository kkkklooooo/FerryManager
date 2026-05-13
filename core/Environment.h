#pragma once
#include "Registry.h"
#include "Organism.h"
#include"Config.h"
#include <utility>

class Environment
{
public:
	Environment(std::pair<int, int> pos,
		float en,
		std::string na,
		int sin);

	std::pair<int, int> Pos;
	float energy;
	float deadOrganismEnergy = 0;
	OrganismType type;
	std::string name;
	int SingleEnvironmentMaxEnergy;
	std::vector<std::string> CanLiveIn;
	std::vector<Reproducable*> Organisms;
	bool canPlant(ReproduceRequest);
	void EnergyExchange(Reproducable* on);
	//void getDeadOrgnismEnergy(float dead) { deadOrganismEnergy += (dead > 0) ? dead : 0.0; }
	static EnvironmentConfig FindEnvironmentConfig(const std::string& name);
	virtual void Update(Weather);
};



class Water : public Environment
{
public:
	Water(std::pair<int, int> pos, float en, float V);
	float Valum;
	void Update(Weather) override;
};

class GressLand :public Environment {
public:
	GressLand(std::pair<int, int> pos, float en);
	void Update(Weather)override;
};
