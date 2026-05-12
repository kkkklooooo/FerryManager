#pragma once
#include"Word.h"
#include<functional>
#include<map>
class MyOperator {
public:
	using Creator = std::function<Animal*( int id, int x, int y, int radius)>;

	//注册函数
	static void register_Animal_Create(std::string name, Creator creator);


	void operator()(Reproducable* a,Reproducable* b);
	Reproducable* operator()(ReproduceRequest &x,int id);
	Reproducable* operator()(int x,int y,int r, std::string n,int id);
	static MyOperator& GetOp();
private:
	//全局唯一注册表
	static std::unordered_map <std::string , Creator>& registry();
};


struct AnimalRegistrator {
	AnimalRegistrator(std::string name, MyOperator::Creator creator) {
		MyOperator::register_Animal_Create(name, std::move(creator));
	}
};
