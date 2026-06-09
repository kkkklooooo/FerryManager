#pragma once

#include"stb_image/stb_image.h"
#include<map>
#include <GL/gl.h>
#include"imgui/imgui.h"
class ImageMaker {
public:
	ImageMaker() = default;
	static std::map<std::string, ImTextureID>& getMap() {
		static std::map<std::string, ImTextureID>fucker;
		return fucker;
	}
};