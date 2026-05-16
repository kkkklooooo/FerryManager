#include"Config.h"

TestConfig* g_GameConf;

void InitGameConfig(const TestConfig& cfg) {
    static TestConfig s_config;
    s_config = cfg;
    g_GameConf = &s_config;
}

WordConfig TestConfig::Check_Word(WordConfig &fu) {
	if (fu.length < 50)fu.length = 50;
	if (fu.width < 50)fu.width = 50;
	return fu;
}


void TestConfig::User_Set_Word(WordConfig &fuck) {
	The_Word = Check_Word(fuck);
}

AnimalConfig TestConfig::Check_Animal(AnimalConfig &fu) {
	if (fu.reproduce_original_rate<0)fu.reproduce_original_rate = Default_Animal_Config.reproduce_original_rate;
	if (fu.reproduce_original_energy < 0)fu.reproduce_original_energy = Default_Animal_Config.reproduce_original_energy;
	if (fu.step_energy_cost < 0)fu.step_energy_cost = Default_Animal_Config.step_energy_cost;
	if (fu.energy_rate < 0)fu.energy_rate = Default_Animal_Config.energy_rate;
	if (fu.eat_intrval_max < 0)fu.eat_intrval_max = Default_Animal_Config.eat_intrval_max;
	if (fu.reproduce_energy_threshold < 0)fu.reproduce_energy_threshold = Default_Animal_Config.reproduce_energy_threshold;
	if(fu.reproduce_energy_cost<0)fu.reproduce_energy_cost= Default_Animal_Config.reproduce_energy_threshold;
	return fu;
}

void TestConfig::User_AddNew_Animal(AnimalConfig& fuck) {
	The_Animals.push_back(Check_Animal(fuck));
}

PlantConfig TestConfig::Check_Plant(PlantConfig& fu) {
	if (fu.reproduce_original_energy < 0)fu.reproduce_original_energy = Default_Plant_Config.reproduce_original_energy;
	if (fu.step_energy_cost < 0)fu.step_energy_cost = Default_Plant_Config.step_energy_cost;
	if (fu.reproduce_energy_threshold < 0)fu.reproduce_energy_threshold = Default_Plant_Config.reproduce_energy_threshold;
	if (fu.reproduce_energy_cost < 0)fu.reproduce_energy_cost = Default_Plant_Config.reproduce_energy_threshold;
	return fu;
}

void TestConfig::User_AddNew_Plant(PlantConfig& fuck) {
	The_Plants.push_back(Check_Plant(fuck));
}

TestConfig& TestConfig::GetTestConfig() {
	return *g_GameConf;
}

Config& Config::GetConfig() {
	static Config The_Config;
	return The_Config;
}
