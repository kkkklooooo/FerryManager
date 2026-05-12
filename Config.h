#pragma once

#include"json.hpp"
#include<string>
#include<vector>

using json = nlohmann::json;


//动物只让改变初始速度，初始能量，名称，食物，初始消耗
struct AnimalConfig {
    std::string name="Animal";
    std::vector<std::string> diet = {};
    float absorb_rate = 0.5f;   
    float loss_rate = 0.9f;
    float reproduce_energy_threshold = 25;
    float reproduce_energy_cost = 10;
    int   reproduce_init_radius = 3;
    int   reproduce_original_rate = -1;
    int   reproduce_original_energy = -1;
    float max_rate=-1;
    float step_energy_cost = -1.0f;
    float overlay_param = 1.4;
    float energy_rate = -1.0f; //速度与能量有关  要通过计算获得
};


//植物只让修改初始能量，初始消耗
struct PlantConfig{
    std::string name="Plant";
    float absorb_rate = 0.5f;   
    float loss_rate = 0.9f;
    float reproduce_energy_threshold = 25;
    float reproduce_energy_cost = 10;
    int   reproduce_init_radius = 3;
    int   reproduce_original_energy = -1;
    float step_energy_cost = - 1.0f;
    float overlay_param = 1.4;
};


//不希望用户能修改我们的环境但是加上好后面更新mod
struct EnvironmentConfig 
{
    std::string name;
    std::vector<std::string> habitat_types;
    int max_Plant;
    float Environment_energy_absorb_rate = 0.01f;
    float Environment_plant_absorb_rate = 0.4f;
    float Environment_step_max_absorb = 2;
    float Environment_single_chunk_max_energy = 50;
};

//应该好修改
struct WordConfig {
    int length = 50;
    int width = 50;
};

class TestConfig {//防止石山爆炸
public:
    WordConfig The_Word;//世界
    std::vector<EnvironmentConfig> The_Environments;//储存环境 (不希望修改）只是增加工作量？
    AnimalConfig Default_Animal_Config;//默认的动物用于补齐参数
    PlantConfig  Default_Plant_Config;
    std::vector<AnimalConfig>The_Animals;
    std::vector<PlantConfig>The_Plants;
    //方法
    void User_Set_Word(WordConfig &fuck);
    void User_AddNew_Animal(AnimalConfig &fuck);
    void User_AddNew_Plant(PlantConfig &fuck);
    AnimalConfig Check_Animal(AnimalConfig &fuck);
    PlantConfig  Check_Plant(PlantConfig &fuck);
    WordConfig   Check_Word(WordConfig &fuck);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WordConfig, length, width)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnimalConfig,
    name, diet, absorb_rate, loss_rate,
    reproduce_energy_threshold, reproduce_energy_cost, reproduce_init_radius,
    reproduce_original_rate, reproduce_original_energy,max_rate,
    step_energy_cost, overlay_param, energy_rate)


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentConfig,
    name,
    habitat_types,
    max_Plant,
    Environment_energy_absorb_rate,
    Environment_plant_absorb_rate,
    Environment_step_max_absorb,
    Environment_single_chunk_max_energy)



NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantConfig,
    name,
    absorb_rate, loss_rate,
    reproduce_energy_threshold,
    reproduce_energy_cost,
    reproduce_init_radius, reproduce_original_energy,
    step_energy_cost, overlay_param)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestConfig, 
    The_Word,
    The_Environments,
    Default_Animal_Config,
    Default_Plant_Config,
    The_Animals,
    The_Plants)

class Config{
    public:
    Config(int length,int width){
        this->length=length;
        this->width=width;
    }
    int length=50;
    int width=50;
    float Environment_energy_absorb_rate=0.01f;
    float Environment_plant_absorb_rate=0.4f;
    float Environment_step_max_absorb=2;
    float Environment_single_chunk_max_energy=50;
    float Organism_animal_absorb_rate=0.5f;
    float Organism_loss_rate=0.9f;
    float Organism_reproduce_energy_threshold=25;
    float Organism_reproduce_energy_cost=10;
    float Organism_step_energy_cost=1;
    float Orgianism_overlay_param=1.4;
    int Plant_init_radius=3;
};


