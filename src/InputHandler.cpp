#include "../Headers/InputHandler.hpp"



float InputHandler::camYaw = 0;
float InputHandler::camPitch = 0;
DirectX::XMFLOAT3 InputHandler::moveDirection = {0,0,0};
bool InputHandler::cursorLock = false;
bool InputHandler::toggleGuiMenu = true;
float InputHandler::mouseXStart = 0;
float InputHandler::mouseYStart = 0;



void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && cursorLock == false)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		mouseXStart = static_cast<float>(xpos);
		mouseYStart = static_cast<float>(ypos);
		cursorLock = true;
	}

	
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		cursorLock = false;
	}
}

void InputHandler::mouseCursorCallback(GLFWwindow* window, double xposin, double yposin)
{
	float xpos = static_cast<float>(xposin);
	float ypos = static_cast<float>(yposin);
	
	if (cursorLock == true)
	{
		float xOffset = xpos - mouseXStart;
		float yOffset = ypos - mouseYStart;
		mouseXStart = xpos;
		mouseYStart = ypos;
		camYaw += xOffset * 0.01f;
		camPitch += yOffset * 0.01f;

	}
} 


void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_F1 && action == GLFW_PRESS)
		toggleGuiMenu = !toggleGuiMenu;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDirection.z = -1.0f;
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDirection.z = 1.0f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDirection.x = 1.0f;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDirection.x = -1.0f;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		moveDirection.y = 1.0f;
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		moveDirection.y = -1.0f;

	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
			case GLFW_KEY_W:
			case GLFW_KEY_S:
				moveDirection.z = 0.0f;
				break;

			case GLFW_KEY_A:
			case GLFW_KEY_D:
				moveDirection.x = 0.0f;
				break;

			case GLFW_KEY_SPACE:
			case GLFW_KEY_LEFT_CONTROL:
				moveDirection.y = 0.0f;
				break;

			default: return;
		}
	}
}

