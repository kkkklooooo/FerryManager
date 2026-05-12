#pragma once

#include"json.hpp"
#include<string>
#include<vector>

using json = nlohmann::json;


// 动物只需要改:初始速度，初始能量，食物，初始名字
struct AnimalConfig {
    std::string name="Animal";
    std::vector<std::string> diet = {};
    int   reproduce_original_rate = -1;
    int   reproduce_original_energy = -1;
    float max_rate=-1;
    float step_energy_cost = -1.0f;
    float energy_rate = -1.0f; // 速度和能量有关  要通过计算来的
};


// 植物只能修改初始能量,初始名字
struct PlantConfig{
    std::string name="Plant";
    int   reproduce_original_energy = -1;
    float step_energy_cost = - 1.0f;
};


// 应用层修改
struct WordConfig {
    int length = 50;
    int width = 50;
};

class TestConfig {// 防止石山爆炸
public:
    WordConfig The_Word;// 世界
    AnimalConfig Default_Animal_Config;// 默认的动物(在内部插值)
    PlantConfig  Default_Plant_Config;
    std::vector<AnimalConfig>The_Animals;
    std::vector<PlantConfig>The_Plants;
    // 方法
    void User_Set_Word(WordConfig &fuck);
    void User_AddNew_Animal(AnimalConfig &fuck);
    void User_AddNew_Plant(PlantConfig &fuck);
    AnimalConfig Check_Animal(AnimalConfig &fuck);
    PlantConfig  Check_Plant(PlantConfig &fuck);
    WordConfig   Check_Word(WordConfig &fuck);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WordConfig, length, width)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnimalConfig,
    name,
    diet,
    reproduce_original_rate,
    reproduce_original_energy,
    max_rate,
    step_energy_cost,
    energy_rate)



NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantConfig,
    name,
    reproduce_original_energy,
    step_energy_cost)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestConfig,
    The_Word,
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
    float Environment_energy_absorb_rate=0.02f;
    float Environment_plant_absorb_rate=0.3f;
    float Environment_step_max_absorb=3;
    float Environment_single_chunk_max_energy=80;
    float Organism_animal_absorb_rate=0.7f;
    float Organism_loss_rate=0.8f;
    float Organism_reproduce_energy_threshold=40;
    float Organism_reproduce_energy_cost=20;
    float Organism_step_energy_cost=0.3f;
    float Orgianism_overlay_param=2.0f;
    int Plant_init_radius=5;
};
