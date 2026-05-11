#pragma once
#include"Organism.h"
#include"MyOperator.h"

//动物的签名都一样！！

class Wolf :public Animal {
public:
	Wolf(int id, int x, int y, int radius)
		:Animal(id,x,y,radius,200,10,1)
	{
		live_environment.push_back(GRESSLEND);
		rate = SetRate();
		name = Wolf_Name;
		diet.push_back(Sheep_Name);
		
	}
	float SetRate() override {
		return 40;
	}
	void  SetRate(Animal* a)override {
		a->rate = a->energy * 0.2;
	}
};

static AnimalRegistrator wolf(Wolf_Name, [](int id, int x, int y, int radius) {return new Wolf(id, x, y, radius); });

class Sheep :public Animal {
public:
	Sheep( int id,int x, int y, int radius)
		:Animal(id,x, y, radius, 150,8,0.8) 
	{
		live_environment.push_back(GRESSLEND);
		rate = SetRate();
		name = Sheep_Name;
		diet.push_back(Plant_Name);
	}
	float SetRate() override {
		return 15;
	}
	void  SetRate(Animal* a) override {
		a->rate = a->energy * 0.1;
	}
};

static AnimalRegistrator sheep(Sheep_Name, [](int id, int x, int y, int radius) {return new Sheep(id, x, y, radius); });