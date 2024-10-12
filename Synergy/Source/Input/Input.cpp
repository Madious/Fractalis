
#include "Snpch.h"
#include "Input.h"
#include "Core/Window.h"


namespace SNG
{

	static Window* s_Window = nullptr;


	void Input::Initialize(Window* window)
	{
		s_Window = window;
	}

	bool Input::IsKeyPress(int keycode)
	{
		SNG_ASSERT(s_Window);
		return glfwGetKey(s_Window->Raw(), keycode) == GLFW_PRESS;
	}

	bool Input::IsKeyRelease(int keycode)
	{
		SNG_ASSERT(s_Window);
		return glfwGetKey(s_Window->Raw(), keycode) == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPress(int keycode)
	{
		SNG_ASSERT(s_Window);
		return glfwGetMouseButton(s_Window->Raw(), keycode) == GLFW_PRESS;
	}

	bool Input::IsMouseButtonRelease(int keycode)
	{
		SNG_ASSERT(s_Window);
		return glfwGetMouseButton(s_Window->Raw(), keycode) == GLFW_RELEASE;
	}

	void Input::SetCursorMode(int mode)
	{
		SNG_ASSERT(s_Window);
		glfwSetInputMode(s_Window->Raw(), GLFW_CURSOR, mode);
	}

	Vec2 Input::GetCursorPosition()
	{
		SNG_ASSERT(s_Window);
		double x, y;
		glfwGetCursorPos(s_Window->Raw(), &x, &y);
		return Vec2((float)x, (float)y);
	}
}
