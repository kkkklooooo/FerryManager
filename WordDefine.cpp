#include"Word.h"
#include"Organism.h"
#include"Environment.h"
#include<algorithm>

World::World(int len, int weigth) {
    Reproducas.push_back(new Plant(id++, 5, 5, 3));
    Reproducas.push_back(new Plant(id++, 4, 5, 3));
    for (int i = 0; i <len; i++) {
        for (int j = 0; j < weight; j++) {
            Environments.push_back(new GressLand(std::make_pair(i, j), 2, 2));
        }
    }
}

void World::AddLeftEnergyRequest(const LeftEnergyRequest &request)
{
    if(request.energy>0) Environments[request.pos.second*weight+request.pos.first]->deadOrganismEnergy+=request.energy;
}

void World::Update()
{
    //杀死死了的东西
    RemoveDeadOrganisms();

    //先移动
    for (auto& i : Reproducas) {
        i->Step();
    }
   
    //环境受天气的影响
    for (auto& i : Environments) {
        i->Update(CurrentWeather);
    }
    //排序
    std::sort(Reproducas.begin(), Reproducas.end(), [](Reproducable* a, Reproducable* b) {
        if (a->Pos.first != b->Pos.first)
            return a->Pos.first < b->Pos.first;
        return a->Pos.second < b->Pos.second;
        });

    //植物和环境交互
    for (auto i : Reproducas) {
        Environments[i->Pos.second*weight+i->Pos.first]->EnergyExchange(i);
    }
    //捕食和生孩子
    for (auto i = Reproducas.begin(); i != Reproducas.end(); i++) {
        for (auto j = i + 0; j != Reproducas.end(); j++) {
            if (isNaber(*i, *j)) {
                PredationOrFuck(*i, *j);
            }
        }
    }
    //繁衍
    World::Reproduce();


}
void World::Reproduce()
{
    for (auto& request : reproduce_requests)
    {
        Reproducas.push_back(ReprodueNewOrganism(request));//调用工厂函数
    }
    reproduce_requests.clear();   // 清空已处理的请求
}

/**
 * @brief 向世界添加一个繁殖请求
 * @param request 繁殖请求结构体
 */
bool  World::AddReproduceRequest(const ReproduceRequest& request)
{
    //能生
    if (Environments[request.pos.first * weight + request.pos.second]->canPlant(request)) {
        reproduce_requests.push_back(request);
        return true;
    }
    return false;
}

/**
 * @brief 移除所有能量为0的生物
 */
void World::RemoveDeadOrganisms()
{
    // 移除能量为0的生物
    Reproducas.erase(std::remove_if(Reproducas.begin(), Reproducas.end(), [&](Reproducable* organism) { 
        if (!(organism->energy>0)){
            Environments[organism->Pos.second*weight+organism->Pos.first]->havePlant--;
            return true;
        }else{
            return false;
        }
        //return !(organism->energy>0); 
    }), Reproducas.end());
}

float World::calculate_overlay(std::pair<int, int> pos)
{
    std::vector<std::pair<int, int>> pos_list={{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,1},{1,-1},{-1,1}};
    int cnt=0;
    for (auto i : pos_list) {
        int x = pos.first + i.first;
        int y = pos.second + i.second;
        if (x >= 0 && x < len && y >= 0 && y < weight) {
            cnt+=Environments[y*weight+x]->havePlant;
        }else{
            cnt+=Environments[pos.second*weight+pos.first]->maxPlant;
        }
    }
    return (float)cnt/(8.0*Environments[pos.second*weight+pos.first]->maxPlant);

}

/**
 * @brief 获取世界单例实例
 * @return World& 世界对象的引用
 */
World& World::GetWorld()
{
    static World Instance(len,weight);
    return Instance;
}
