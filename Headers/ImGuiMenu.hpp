#pragma once
#include "Definitions.hpp"
#include <DirectXMath.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_dx11.h>

#include <string>
#include <iostream>

struct ImGuiMenuData
{
	std::string objFile;
	bool showMenu = true;
	DirectX::XMVECTOR camPos;
};

class ImGuiMenu
{
public:
	static bool Initialize(GLFWwindow* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	static void Update(ImGuiMenuData& menuData);
	static void CleanUp();
};
