
#include "Snpch.h"
#include "Window.h"
#include "Input/Input.h"
#include "API/VulkanContext.h"
#include "Editor/Editor.h"


namespace SNG
{

	static void GLFWResizeCallback(GLFWwindow* window, int width, int height)
	{
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		Window* userPtr = (Window*)glfwGetWindowUserPointer(window);
		userPtr->m_ResizeCallback();
	}



	Window::Window(const Vec2& size, const std::string& title)
	{
		SNG_TRACE("Window creation...");
		m_ResizeCallback = []() {};

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow((int)size.x, (int)size.y, title.c_str(), nullptr, nullptr);
		glfwSetFramebufferSizeCallback(m_Window, GLFWResizeCallback);
		glfwSetWindowUserPointer(m_Window, this);

		glfwSetErrorCallback([](int code, const char* desc)
		{
			SNG_ERROR("[GLFW ERROR] ({}) : {}", code, desc);
		});

		Input::Initialize(this);
		VulkanContext::Initialize(this);
		Editor::Initialize(this);
	}

	Window::~Window()
	{
		Editor::Shutdown();
		VulkanContext::Shutdown();
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	Vec2 Window::GetSize() const
	{
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);
		return Vec2((float)width, (float)height);
	}

	bool Window::IsClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}

	void Window::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_Window, title.c_str());
	}

	GLFWwindow* Window::Raw() const
	{
		return m_Window;
	}

	void Window::SetResizeCallback(std::function<void()> callback)
	{
		m_ResizeCallback = callback;
	}
}
