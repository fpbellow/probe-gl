#pragma once
#include <GLFW/glfw3.h>
#include <DirectXMath.h>

class InputHandler
{
public:
	
	static bool toggleGuiMenu;
	static float camYaw;
	static float camPitch;
	static DirectX::XMFLOAT3 moveDirection;

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	static bool cursorLock;
	static float mouseXStart;
	static float mouseYStart;
};