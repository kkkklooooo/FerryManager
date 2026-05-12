#include "Organism.h"
#include<cstdio>
#include <cassert>
#include "Registry.h"
#include "World.h"
#include "Environment.h"
#include <algorithm>

Environment::Environment(std::pair<int, int> pos,
						 float en,
						 std::string na,
						 int sin,
						 int mp)
	: Pos(pos), energy(en), type(ENVIRONMENT), name(na), SingleEnvironmentMaxEnergy(sin), maxPlant(mp), havePlant(0)
{}


bool Environment::canPlant(ReproduceRequest a)
{
	if (havePlant < maxPlant)
	{
		if (std::find(CanLiveIn.begin(), CanLiveIn.end(), a.name) != CanLiveIn.end())
		{
			if(a.type==PLANT)havePlant++;//植物才++ 因为植物不会动
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
	//只有死后才能add
	if (on->type == PLANT && on->active)
	{ // 植物吸收土地 
	  // @ TODO 修改参数
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

void Environment::Update(Weather)
{
	float gain = deadOrganismEnergy * World::GetWorld().conf.Organism_loss_rate; // 增加尸体上的能量
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
	CanLiveIn.push_back("Plant");
	CanLiveIn.push_back("Animal");
	CanLiveIn.push_back("Sheep");
	CanLiveIn.push_back("Wolf");
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
