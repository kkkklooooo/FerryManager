#include"Organism.h"
#include<cassert>
#include "Registry.h"
#include"Word.h"
#include"Environment.h"
#include<algorithm>


Environment::Environment(std::pair<int, int> pos,
	float en,
	EnvironmentType na,
	int mp)
	:Pos(pos)
	,energy(en)
	,type(ENVIRONMENT)
	,name(na)
	,maxPlant(mp)
	,havePlant(0)
{}
bool Environment::canPlant(ReproduceRequest a) {
	if (havePlant < maxPlant) {
		if (std::find(CanLiveIn.begin(), CanLiveIn.end(), a.name) != CanLiveIn.end()) {
			havePlant++;
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;

}

void Environment::EnergyExchange(Reproducable* on) {
	// if(energy<SingleEnvironmentMaxEnergy) energy +=(  on->step_energy_cost * EnvironmentEnergyAbsorbRate*lossRate);//土地吸收粪便或者尸体 cost已经在step去除了
	if (on->type == PLANT&&on->active) {//植物吸收土地
		float abs=std::min( World::GetWorld().conf.Environment_plant_absorb_rate * energy,World::GetWorld().conf.Environmrnt_step_max_absorb/World::GetWorld().conf.Organism_loss_rate);
		abs=abs*World::GetWorld().conf.Organism_loss_rate;
		on->energy +=abs;
		energy -= abs;
	}
}


void Environment::Update(Weather)
{
	float gain=deadOrganismEnergy*World::GetWorld().conf.Organism_loss_rate;//增加尸体上的能量
	energy += gain;
	deadOrganismEnergy=0;
}

Water::Water(std::pair<int, int> pos, float en, int mp, float V)
	:Environment(pos, en, WATER, mp), Valum(V)
{}

void Water::Update(Weather sky) {
	switch (sky) {
		case SUN:
			Valum -= 0.1;
			if(energy<World::GetWorld().conf.Environment_single_chunk_max_energy)energy += 0.1;
			break;
		default:
			break;
	}
	Environment::Update(sky);
}

GressLand::GressLand(std::pair<int, int> pos,float en,int mp) 
	:Environment(pos, en, GRESSLEND, mp)
{
	CanLiveIn.push_back(Plant_Name);
}

void GressLand::Update(Weather sky) {
	switch (sky) {
	case SUN:
		if(World::GetWorld().conf.Environment_single_chunk_max_energy>=energy)energy += 2;
		break;
	default:
		break;
	}
	Environment::Update(sky);
}

