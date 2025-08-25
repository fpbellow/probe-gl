#pragma once
#include <GLFW/glfw3.h>


#include <cstdint>
#include <string>
#include <chrono>

struct GLFWWindow;

class Application
{

public:
	Application(const std::string& title);
	virtual ~Application();

	void Run();


protected:
	virtual bool Initialize();
	virtual bool Load() = 0;
	virtual void CleanUp();
	virtual void Render() = 0;
	virtual void Update();

	[[nodiscard]] GLFWwindow* GetWindow() const;
	[[nodiscard]] int32_t GetWindowWidth() const;
	[[nodiscard]] int32_t GetWindowHeight() const;

	GLFWwindow* _window = nullptr;
	int32_t _width = 0;
	int32_t _height = 0;
	float _deltaTime = 0.016f;

private:
	std::chrono::high_resolution_clock::time_point _currentTime;
	std::string _title;
};