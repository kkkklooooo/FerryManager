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
{ // ��������

public:
	Environment(std::pair<int, int> pos,
				float en,
				std::string na,
				int sin, 
				int mp);
	std::pair<int, int> Pos;
	float energy;//��ʼ����
	float deadOrganismEnergy=0;//��ǰ�������������ʣ������,δ�����յ�
	OrganismType type;
	std::string name;
	int SingleEnvironmentMaxEnergy;//ˮ�Ͳݵص��������޿϶���ͬ��
	std::vector<std::string> CanLiveIn;//�ܻ���
	int maxPlant;//������ж���
	int havePlant;//�����ж���

	std::vector<Reproducable*> Organisms;


	bool canPlant(ReproduceRequest);//����
	void EnergyExchange(Reproducable* on);//��������
	void getDeadOrgnismEnergy(float dead) { deadOrganismEnergy += (dead > 0) ? dead : 0.0; }
	virtual void Update(Weather);//����
};

class Water : public Environment
{
public:
	Water(std::pair<int, int> pos, float en, int mp, float V);
	float Valum; // ˮ�Ķ���
	void Update(Weather) override;
};

class GressLand :public Environment {
public:
	GressLand(std::pair<int, int> pos,
		float en,
		int maxPlant);
	void Update(Weather)override;
};
