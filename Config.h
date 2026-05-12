#pragma once

#include"json.hpp"
#include<string>
#include<vector>

using json = nlohmann::json;


//����ֻ�øı��ʼ�ٶȣ���ʼ���������ƣ�ʳ���ʼ����
struct AnimalConfig {
    std::string name="Animal";
    std::vector<std::string> diet = {};
    int   reproduce_original_rate = -1;
    int   reproduce_original_energy = -1;
    float max_rate=-1;
    float step_energy_cost = -1.0f;
    float energy_rate = -1.0f; //�ٶ��������й�  Ҫͨ��������

};

struct EnvironmentConfig{
    std::string name;
    std::vector<std::string> CanLive;
};

//ֲ��ֻ���޸ĳ�ʼ��������ʼ����
struct PlantConfig{
    std::string name="Plant";
    int   reproduce_original_energy = -1;
    float step_energy_cost = - 1.0f;
};




//Ӧ�ú��޸�
struct WordConfig {
    int length = 50;
    int width = 50;
};

class TestConfig {//��ֹʯɽ��ը
public:
    WordConfig The_Word;//����
    AnimalConfig Default_Animal_Config;//Ĭ�ϵĶ������ڲ������
    PlantConfig  Default_Plant_Config;
    std::vector<EnvironmentConfig>The_Environments;
    std::vector<AnimalConfig>The_Animals;
    std::vector<PlantConfig>The_Plants;
    //����
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentConfig,
    name,
    CanLive)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantConfig,
    name,
    reproduce_original_energy,
    step_energy_cost)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestConfig, 
    The_Word,
    Default_Animal_Config,
    Default_Plant_Config,
    The_Environments,
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


