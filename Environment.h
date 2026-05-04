#pragma once
#include "Registry.h"
#include "Organism.h"
#include <utility>
const float EnvironmentEnergyAbsorbRate=0.01;
const float PlantAbsortRate = 0.4;
const float StepMaxAbsorb = 2;
const float SingleEnvironmentMaxEnergy = 50;

class Environment
{ // 环境大类

public:
	Environment(std::pair<int, int> pos,
				float en,
				EnvironmentType na,
				int mp);
	std::pair<int, int> Pos;
	float energy;//初始能量
	OrganismType type;
	EnvironmentType name;
	std::vector<OrganismName> CanLiveIn;//能活着
	int maxPlant;//草最多有多少
	int havePlant;//现在有多少
	bool canPlant(ReproduceRequest);//能生
	void EnergyExchange(Reproducable* on);//能量交换
	virtual void Update(Weather)=0;//更新
};

class Water : public Environment
{
public:
	Water(std::pair<int, int> pos, float en, int mp, float V);
	float Valum; // 水的多少
	void Update(Weather) override;
};

class GressLand :public Environment {
public:
	GressLand(std::pair<int, int> pos,
		float en,
		int maxPlant);
	void Update(Weather)override;
};
