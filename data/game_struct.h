#pragma once
#include<string>
#include<vector>
#include"json.hpp"
struct gameData
{
	std::vector<std::string>names;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(gameData,
	names)


