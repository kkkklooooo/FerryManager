#pragma once
#include"Registry.h"
#include"Organism.h"
#include"Environment.h"
#include"Config.h"
class World
{
    // �������ķ�ֳ�����б�
    std::vector<ReproduceRequest> reproduce_requests;
    // ���пɷ�ֳ����ָ���б�
    std::vector<Reproducable*> Reproducas;
    //���л�������
    std::vector<Environment*>Environments;

public:
    TestConfig& game_cof;
    Config conf;
    Weather CurrentWeather;
    World(Config&Conf, TestConfig& Game_conf);
    void AddLeftEnergyRequest(const LeftEnergyRequest& request);
    // ��������״̬�����绷���仯�ȣ�
    void Update();
    // �������з�ֳ��������������
    void Reproduce();
    // ����������һ����ֳ����
    bool AddReproduceRequest(const ReproduceRequest& request);
    // �Ƴ���������Ϊ0��active == false��������
    void RemoveDeadOrganisms();
    float calculate_overlay(std::pair<int, int> pos);
    void Reset();
    // ��ȡ���絥��ʵ��
    static World& GetWorld();
    //ֻ������ ���������Ĵ���
    const std::vector<Reproducable*>& GetReproducas() const { return Reproducas; }
    const std::vector<Environment*>& GetEnvironments() const { return Environments; }
    int GetWidth() const { return conf.width; }
    int GetHeight() const { return conf.length; }
};
