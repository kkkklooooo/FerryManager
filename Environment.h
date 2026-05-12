#pragma once
#include "Registry.h"
#include "Organism.h"
#include"Config.h"
#include <utility>
// const float EnvironmentEnergyAbsorbRate=0.01;
// const float PlantAbsortRate = 0.4;
// const float StepMaxAbsorb = 2;
// const float SingleEnvironmentMaxEnergy = 50;

class Environment
{ // 环境大类

public:
	Environment(std::pair<int, int> pos,
				float en,
				std::string na,
				int sin, 
				int mp);

	std::pair<int, int> Pos;
	float energy;//初始能量
	float deadOrganismEnergy=0;//当前区块死亡生物的剩余能量,未被吸收的
	OrganismType type;
	std::string name;
	int SingleEnvironmentMaxEnergy;//水和草地的能量上限肯定不同的
	std::vector<std::string> CanLiveIn;//能活着
	int maxPlant;//草最多有多少
	int havePlant;//现在有多少
	bool canPlant(ReproduceRequest);//能生
	void EnergyExchange(Reproducable* on);//能量交换
	void getDeadOrgnismEnergy(float dead) { deadOrganismEnergy += (dead > 0) ? dead : 0.0; }

	virtual void Update(Weather);//更新
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
