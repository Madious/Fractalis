
#pragma once
#include "Core/Core.h"
#include "Layer.h"
#include "Window.h"


namespace SNG
{

	class Application
	{
	public:

		static Application* Get() { return s_Instance; }

		Application(const Vec2& windowSize, const std::string& windowTitle);
		~Application();
		void Run();

		template<typename T>
		void PushLayer()
		{
			SNG_ASSERT(SNG_IS_BASE_OF(T, Layer));
			m_LayerStack.emplace_back(MakeRef<T>());
		}

		void SubmitFunction(const std::function<void()>& func);
		Ref<Window> GetWindow() const { return m_Window; }

	private:

		static Application* s_Instance;

		Ref<Window> m_Window;
		std::vector<Ref<Layer>> m_LayerStack;
		std::vector<std::function<void()>> m_SubmitFunctions;
	};

	Application* CreateApplication();
}
