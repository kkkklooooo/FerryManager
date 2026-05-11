#pragma once
#include"Organism.h"
#include"MyOperator.h"

//动物的签名都一样！！

class Wolf :public Animal {
public:
	Wolf(int id, int x, int y, int radius)
		:Animal(id,x,y,radius,200,10,1)
	{
		rate = SetRate();
		name = Wolf_Name;
		diet.push_back(Sheep_Name);
		
	}
	float SetRate() override {

	}
	void  SetRate(Animal* a)override {

	}
};

static AnimalRegistrator Wole(Wolf_Name, [](int id, int x, int y, int radius) {return new Wolf(id, x, y, radius); });

class Sheep :public Animal {
public:
	Sheep( int id,int x, int y, int radius)
		:Animal(id,x, y, radius, 150,8,0.8) 
	{
		rate = SetRate();
		name = Sheep_Name;
		diet.push_back(Plant_Name);
	}
	float SetRate() override {

	}
	void  SetRate(Animal* a) override {

	}
};

static AnimalRegistrator Wole(Sheep_Name, [](int id, int x, int y, int radius) {return new Sheep(id, x, y, radius); });