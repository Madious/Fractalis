
#pragma once
#include "Core/Window.h"


namespace SNG
{

	class Editor
	{
	public:

		static void Initialize(Window* window);
		static void Shutdown();

		static void SetMenuBarCallback(const std::function<void()>& func);
		static void BeginFrame();
		static void EndFrame();
	};
}
