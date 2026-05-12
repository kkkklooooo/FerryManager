#pragma once
#include"Organism.h"
#include"MyOperator.h"

//动物的签名都一样！！




class Wolf :public Animal {
public:
	Wolf(int id, int x, int y, int radius)
		:Animal(id,x,y,radius,100,50,0.01f)
	{
		rate = SetRate();
		name = "Wolf";
		diet.push_back("Sheep");
		
	}
	float SetRate() override {
		return 0.5f;
	}
	void  SetRate(Animal* a)override {

	}
};

static AnimalRegistrator wolf("Wolf", [](int id, int x, int y, int radius) {return new Wolf(id, x, y, radius); });

class Sheep :public Animal {
public:
	Sheep( int id,int x, int y, int radius)
		:Animal(id,x, y, radius, 50,20,0.01f) 
	{
		rate = SetRate();
		name = "Sheep";
		diet.push_back("Plant");
	}
	float SetRate() override {
		return 0.5f;
	}
	void  SetRate(Animal* a) override {

	}
};

static AnimalRegistrator sheep("Sheep", [](int id, int x, int y, int radius) {return new Sheep(id, x, y, radius); });