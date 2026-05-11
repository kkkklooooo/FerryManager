#pragma once
#include"Word.h"
#include<functional>
#include<map>
class MyOperator {
public:
	using Creator = std::function<Animal*( int id, int x, int y, int radius)>;

	//注册函数
	static void register_Animal_Create(OrganismName name, Creator creator);


	void operator()(Reproducable* a,Reproducable* b);
	Reproducable* operator()(ReproduceRequest &x,int id);
	static MyOperator& GetOp();
private:
	//全局唯一注册表
	static std::unordered_map <OrganismName, Creator>& registry();
};


struct AnimalRegistrator {
	AnimalRegistrator(OrganismName name, MyOperator::Creator creator) {
		MyOperator::register_Animal_Create(name, std::move(creator));
	}
};
