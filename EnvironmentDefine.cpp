#include "Organism.h"
#include<cstdio>
#include <cassert>
#include "Registry.h"
#include "World.h"
#include "Environment.h"
#include <algorithm>
#include <stdexcept>

Environment::Environment(std::pair<int, int> pos,
						 float en,
						 std::string na,
						 int sin,
						 int mp)
	: Pos(pos), energy(en), type(ENVIRONMENT), name(na), SingleEnvironmentMaxEnergy(sin), maxPlant(mp), havePlant(0)
{
	CanLiveIn = Environment::FindEnvironmentConfig(na).CanLive;
}


bool Environment::canPlant(ReproduceRequest a)
{
	if (havePlant < maxPlant)
	{
		if (std::find(CanLiveIn.begin(), CanLiveIn.end(), a.name) != CanLiveIn.end())
		{
			if(a.type==PLANT) havePlant++; // 植物++ 因为植物不会动
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void Environment::EnergyExchange(Reproducable *on)
{
	//ֻ���������add
	if (on->type == PLANT && on->active)
	{ // ֲ���������� 
	  // @ TODO �޸Ĳ���
		float abs = std::min(World::GetWorld().conf.Environment_plant_absorb_rate * energy, World::GetWorld().conf.Environment_step_max_absorb / World::GetWorld().conf.Organism_loss_rate);
		abs = abs * World::GetWorld().conf.Organism_loss_rate;
		on->energy += abs;
		// assert(on->energy>=-100);
		energy -= abs;
		
	}
	if (energy < 0)
		{
			printf("error");
		}
}

EnvironmentConfig Environment::FindEnvironmentConfig(const std::string& name) {
	auto& animals = TestConfig::GetTestConfig().The_Environments;
	for (auto& a : animals) {
		if (a.name == name) return a;
	}
	throw std::runtime_error("Unknown environment: " + name);
}

void Environment::Update(Weather)
{
	float gain = deadOrganismEnergy * World::GetWorld().conf.Organism_loss_rate; // ����ʬ���ϵ�����
	if(energy<World::GetWorld().conf.Environment_single_chunk_max_energy*2) energy += std::min(gain,World::GetWorld().conf.Environment_single_chunk_max_energy-energy);
	
	deadOrganismEnergy = 0;
}

Water::Water(std::pair<int, int> pos, float en, int mp, float V)
	: Environment(pos, en, "Water", 100, mp), Valum(V)
{
}

void Water::Update(Weather sky)
{
	switch (sky)
	{
	case SUN:
		Valum -= 0.1;
		if (energy < World::GetWorld().conf.Environment_single_chunk_max_energy)
			energy += 0.1;
		break;
	default:
		break;
	}
	Environment::Update(sky);
}

GressLand::GressLand(std::pair<int, int> pos, float en, int mp)
	: Environment(pos, en, "GressLand", 200, mp)
{
	
}

void GressLand::Update(Weather sky)
{
	switch (sky)
	{
	case SUN:
		if (World::GetWorld().conf.Environment_single_chunk_max_energy >= energy)
			energy += 2;
		break;
	default:
		break;
	}
	Environment::Update(sky);
}
