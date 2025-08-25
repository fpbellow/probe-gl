#include "../Headers/ImGuiMenu.hpp"

bool ImGuiMenu::Initialize(GLFWwindow* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& _io = ImGui::GetIO(); (void)_io;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMinSize = ImVec2(200, 400);
	ImGui_ImplGlfw_InitForOther(window, true);
	ImGui_ImplDX11_Init(device, deviceContext);

	return true;
}

void ImGuiMenu::Update(ImGuiMenuData& menuData)
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	if (menuData.showMenu)
	{
		ImGui::Begin("Global Illumination project");
		ImGui::Text("GLTF File: %s", menuData.objFile.c_str());

		ImGui::Separator();
		ImGui::Text("\n");
		ImGui::Text("Press F1 to Open/Hide Menu.");
		ImGui::Text("Hold Right Click to rotate camera.");
		ImGui::Separator();
		ImGui::Text("\n");
		ImGui::Text("\n");
		ImGui::Text("CamPos X: %f", DirectX::XMVectorGetX(menuData.camPos));
		ImGui::Text("CamPos Y: %f", DirectX::XMVectorGetY(menuData.camPos));
		ImGui::Text("CamPos Z: %f", DirectX::XMVectorGetZ(menuData.camPos));
		
		ImGui::End();
	}
	
}

void ImGuiMenu::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}