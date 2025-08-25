#include "../Headers/Application.hpp"

#include <iostream>

Application::Application(const std::string& title)
	: _title(title)
{
}

Application::~Application()
{
	Application::CleanUp();
}

bool Application::Initialize()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW: Failed to initialize. \n";
		return false;
	}

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
	_width = static_cast<int32_t>(videoMode->width * 0.8f);
	_height = static_cast<int32_t>(videoMode->height * 0.8f);

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	_window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);

	if (_window == nullptr)
	{
		std::cerr << "GLFW: Failed to create a window. \n";
		CleanUp();
		return false;
	}

	glfwSetWindowUserPointer(_window, this);

	_currentTime = std::chrono::high_resolution_clock::now();
	return true;
}

void Application::CleanUp()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Application::Run()
{
	if (!Initialize())
	{
		return;
	}

	if (!Load())
	{
		return;
	}

	while (!glfwWindowShouldClose(_window))
	{
		Update();
		Render();
	}
}

GLFWwindow* Application::GetWindow() const
{
	return _window;
}

int32_t Application::GetWindowWidth() const
{
	return _width;
}

int32_t Application::GetWindowHeight() const
{
	return _height;
}

void Application::Update()
{
	auto oldTime = _currentTime;
	_currentTime = std::chrono::high_resolution_clock::now();
	
	std::chrono::duration<double, std::milli> timeSpan = (_currentTime - oldTime);
	_deltaTime = static_cast<float>(timeSpan.count() / 1000.0);
	glfwPollEvents();
}