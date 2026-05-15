#pragma once
#include<boids/Genes.h>
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
    float energy_rate = -1.0f;
    int   eat_intrval_max = -1;
    float reproduce_energy_threshold = -1; // 繁殖所需能量阈值，-1=使用全局默认
    float reproduce_energy_cost = -1;      // 繁殖消耗能量，-1=使用全局默认
};

struct EnvironmentConfig{
    std::string name;
    std::vector<std::string> CanLive;
};


// 植物只能修改初始能量,初始名字
struct PlantConfig{
    std::string name="Plant";
    int   reproduce_original_energy = -1;
    float step_energy_cost = - 1.0f;
    float reproduce_energy_threshold = -1; // 繁殖所需能量阈值，-1=使用全局默认
    float reproduce_energy_cost = -1;      // 繁殖消耗能量，-1=使用全局默认
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
    std::vector<EnvironmentConfig>The_Environments;
    std::vector<AnimalConfig>The_Animals;
    std::vector<PlantConfig>The_Plants;
    //����
    static TestConfig& GetTestConfig();
    void User_Set_Word(WordConfig &fuck);
    void User_AddNew_Animal(AnimalConfig &fuck);
    void User_AddNew_Plant(PlantConfig &fuck);
    AnimalConfig Check_Animal(AnimalConfig &fuck);
    PlantConfig  Check_Plant(PlantConfig &fuck);
    WordConfig   Check_Word(WordConfig &fuck);
};

void InitGameConfig(const TestConfig& cfg);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WordConfig, length, width)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnimalConfig,
    name,
    diet,
    reproduce_original_rate,
    reproduce_original_energy,
    max_rate,
    step_energy_cost,
    energy_rate,
    eat_intrval_max,
    reproduce_energy_threshold,
    reproduce_energy_cost)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentConfig,
    name,
    CanLive)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantConfig,
    name,
    reproduce_original_energy,
    step_energy_cost,
    reproduce_energy_threshold,
    reproduce_energy_cost)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestConfig,
    The_Word,
    Default_Animal_Config,
    Default_Plant_Config,
    The_Environments,
    The_Animals,
    The_Plants)

class Config{
    public:
 /*   Config(int length,int width){
        this->length=length;
        this->width=width;
    }
    int length=50;*/
    int width=50;
    float Environment_energy_absorb_rate=0.01f;
    float Environment_plant_absorb_rate=0.2f;
    float Environment_step_max_absorb=1.5f;
    float Environment_single_chunk_max_energy=25;
    float Organism_animal_absorb_rate=0.4f;
    float Organism_loss_rate=0.85f;
    float Organism_reproduce_energy_threshold=22;
    float Organism_reproduce_energy_cost=10;
    float Organism_step_energy_cost=0.2f;
    float Organism_overlay_param=1.0f;
    int Plant_init_radius=3;
    int Organism_interact_radius=2;
    int max_organisms_per_cell=4;
    static Config& GetConfig();
};
