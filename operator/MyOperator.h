#pragma once
#include"World.h"
#include<functional>
#include<map>
class MyOperator {
public:
	using Creator = std::function<Animal*( int id, int x, int y, int radius)>;
	using PlantCreator = std::function<Plant* (int id, int x, int y, int radius)>; //注册官方植物的

	//ע�ắ��
	static void register_Animal_Create(std::string name, Creator creator);
	static void register_Plant_Create(std::string name, PlantCreator creator);

	void operator()(Reproducable* a,Reproducable* b);
	Reproducable* operator()(ReproduceRequest &x,int id);
	Reproducable* operator()(int x,int y,int r, std::string n,int id);
	static MyOperator& GetOp();
private:
	//ȫ��Ψһע���
	static std::unordered_map <std::string , Creator>& registry();
	static std::unordered_map <std::string, PlantCreator>& Plantregistry();
};

struct PlantRegistrator {
	PlantRegistrator(std::string name, MyOperator::PlantCreator creator) {
		MyOperator::register_Plant_Create(name, std::move(creator));
	}
};

struct AnimalRegistrator {
	AnimalRegistrator(std::string name, MyOperator::Creator creator) {
		MyOperator::register_Animal_Create(name, std::move(creator));
	}
};
