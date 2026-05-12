#pragma once

#include"json.hpp"
#include<string>
#include<vector>

using json = nlohmann::json;


//����ֻ�øı��ʼ�ٶȣ���ʼ���������ƣ�ʳ���ʼ����
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
    float energy_rate = -1.0f; //�ٶ��������й�  Ҫͨ��������
};


//ֲ��ֻ���޸ĳ�ʼ��������ʼ����
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


//��ϣ���û����޸����ǵĻ������Ǽ��Ϻú������mod
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

//Ӧ�ú��޸�
struct WordConfig {
    int length = 50;
    int width = 50;
};

class TestConfig {//��ֹʯɽ��ը
public:
    WordConfig The_Word;//����
    std::vector<EnvironmentConfig> The_Environments;//���滷�� (��ϣ���޸ģ�ֻ�����ӹ�������
    AnimalConfig Default_Animal_Config;//Ĭ�ϵĶ������ڲ������
    PlantConfig  Default_Plant_Config;
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
    float Environment_energy_absorb_rate=0.02f;     // 环境从尸体回收能量的速率
    float Environment_plant_absorb_rate=0.3f;       // 植物每步吸收环境能量的比例
    float Environment_step_max_absorb=3;            // 单步最大能量交换
    float Environment_single_chunk_max_energy=80;   // 单块环境能量上限
    float Organism_animal_absorb_rate=0.7f;          // 捕食者吸收猎物能量的比例
    float Organism_loss_rate=0.8f;                   // 能量传递效率 (1-loss)
    float Organism_reproduce_energy_threshold=40;   // 繁殖能量阈值
    float Organism_reproduce_energy_cost=20;        // 繁殖消耗
    float Organism_step_energy_cost=0.3f;            // 每步基础代谢消耗
    float Orgianism_overlay_param=2.0f;             // 拥挤容忍参数 (越高越容忍)
    int Plant_init_radius=5;                        // 植物初始繁殖半径
};


