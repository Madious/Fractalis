
#pragma once
#include "Core.h"


namespace SNG
{

	class Window
	{
	public:

		Window(const Vec2& size, const std::string& title);
		~Window();

		void OnUpdate();
		Vec2 GetSize() const;
		bool IsClose() const;
		void SetTitle(const std::string& title);
		GLFWwindow* Raw() const;

		void SetResizeCallback(std::function<void()> callback);

	private:

		GLFWwindow* m_Window;
		std::function<void()> m_ResizeCallback;

		friend void GLFWResizeCallback(GLFWwindow*, int, int);
	};
}
