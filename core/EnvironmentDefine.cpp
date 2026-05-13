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
						 int sin)
	: Pos(pos), energy(en), type(ENVIRONMENT), name(na), SingleEnvironmentMaxEnergy(sin)
{
	CanLiveIn = Environment::FindEnvironmentConfig(na).CanLive;
}


bool Environment::canPlant(ReproduceRequest a)
{
	int sameSpecies = 0;
	for (auto* org : Organisms) {
		if (org && org->name == a.name) sameSpecies++;
	}
	if (sameSpecies >= Config::GetConfig().max_organisms_per_cell)
		return false;

	if (std::find(CanLiveIn.begin(), CanLiveIn.end(), a.name) != CanLiveIn.end())
		return true;

	return false;
}

void Environment::EnergyExchange(Reproducable *on)
{
	if (on->type == PLANT && on->active && energy > 0)
	{
		float abs = std::min(Config::GetConfig().Environment_plant_absorb_rate * energy, Config::GetConfig().Environment_step_max_absorb / World::GetWorld().conf.Organism_loss_rate);
		abs = abs * Config::GetConfig().Organism_loss_rate;
		abs = std::min(abs, energy);
		on->energy += abs;
		this->energy -= abs;
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
	float gain = deadOrganismEnergy * World::GetWorld().conf.Organism_loss_rate;
	if(energy< Config::GetConfig().Environment_single_chunk_max_energy*2) energy += std::min(gain, Config::GetConfig().Environment_single_chunk_max_energy-energy);
	deadOrganismEnergy = 0;
}

Water::Water(std::pair<int, int> pos, float en, float V)
	: Environment(pos, en, "Water", 100), Valum(V)
{
}

void Water::Update(Weather sky)
{
	switch (sky)
	{
	case SUN:
		Valum -= 0.1;
		if (energy < Config::GetConfig().Environment_single_chunk_max_energy)
			energy += 0.1;
		break;
	default:
		break;
	}
	Environment::Update(sky);
}

GressLand::GressLand(std::pair<int, int> pos, float en)
	: Environment(pos, en, "GressLand", 200)
{
}

void GressLand::Update(Weather sky)
{
	switch (sky)
	{
	case SUN:
		if (Config::GetConfig().Environment_single_chunk_max_energy >= energy)
			energy += 0.8f;
		break;
	default:
		break;
	}
	Environment::Update(sky);
}
