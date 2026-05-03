#include"Word.h"
#include"Organism.h"
#include"Environment.h"
#include<algorithm>

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
        return ((a->Pos.first == b->Pos.first) ?
            (a->Pos.second >= b->Pos.second) :
            (a->Pos.first >= b->Pos.first));
        });

    //植物和环境交互
    for (auto i : Reproducas) {
        Environments[i->Pos.first][i->Pos.second].EnergyExchange(i);
    }
    //捕食和生孩子
    for (auto i = Reproducas.begin(); i != Reproducas.end(); i++) {
        for (auto j = i + 1; j != Reproducas.end(); j++) {
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
void World::AddReproduceRequest(const ReproduceRequest& request)
{
    reproduce_requests.push_back(request);
}

/**
 * @brief 移除所有能量为0的生物
 */
void World::RemoveDeadOrganisms()
{
    // 移除能量为0的生物
    Reproducas.erase(std::remove_if(Reproducas.begin(), Reproducas.end(), [](Reproducable* organism) { return !organism->energy>0; }), Reproducas.end());
}

/**
 * @brief 获取世界单例实例
 * @return World& 世界对象的引用
 */
World& World::GetWorld()
{
    static World Instance;
    return Instance;
}
