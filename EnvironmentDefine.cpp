#include"Organism.h"
#include "Registry.h"
#include"Word.h"
#include"Environment.h"


Environment::Environment(std::pair<int, int> pos,
	float en,
	EnvironmentType na,
	int mp)
	:Pos(pos)
	,energy(en)
	,type(ENVIRONMENT)
	,name(na)
	,maxPlant(mp)
{}

void Environment::EnergyExchange(Reproducable* on) {
	energy += on->step_energy_cost * EnvironmentEnergyAbsorbRate*lossRate;//土地吸收粪便或者尸体 cost已经在step去除了
	if (on->type == PLANT) {//植物吸收土地
		on->energy += PlantAbsortRate * energy*lossRate;
		energy -= PlantAbsortRate * energy;
	}
}

Water::Water(std::pair<int, int> pos, float en, int mp, float V)
	:Environment(pos, en, WATER, mp), Valum(V)
{}

void Water::Update(Weather sky) {
	switch (sky) {
		case SUN:
			Valum -= 0.1;
			energy += 0.5;
			break;
		default:
			break;
	}
}

GlassLand::GlassLand(std::pair<int, int> pos,float en,int mp) 
	:Environment(pos, en, GRASSLEND, mp)
{}

void GlassLand::Update(Weather sky) {
	switch (sky) {
	case SUN:
		energy += 1;
		break;
	default:
		break;
	}
}

