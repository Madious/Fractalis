
#pragma once
#include "Core/Core.h"


namespace SNG
{

	class Window;

	class Input
	{
	public:

		static void Initialize(Window* window);

		static bool IsKeyPress(int keycode);
		static bool IsKeyRelease(int keycode);
		static bool IsMouseButtonPress(int keycode);
		static bool IsMouseButtonRelease(int keycode);
		static void SetCursorMode(int mode);
		static Vec2 GetCursorPosition();
	};
}
