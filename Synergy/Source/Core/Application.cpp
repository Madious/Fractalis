
#include "Snpch.h"
#include "Application.h"
#include "API/VulkanContext.h"
#include "Editor/Editor.h"
#include "Random/Random.h"


namespace SNG
{

	Application* Application::s_Instance = nullptr;


	static void WindowResizeCallback()
	{
		Application::Get()->SubmitFunction([]()
		{
			VulkanContext::Wait();
			VulkanContext::RecreateSwapchain();
		});
	}


	Application::Application(const Vec2& windowSize, const std::string& windowTitle)
	{
		s_Instance = this;
		Log::Initialize();
		SNG_INFO("Engine starting");

		m_Window = MakeRef<Window>(windowSize, windowTitle);
		m_Window->SetResizeCallback(WindowResizeCallback);

		Random::Initialize();
		SNG_INFO("Engine ready");
	}

	Application::~Application()
	{
		m_LayerStack.clear();

		for (auto& func : m_SubmitFunctions)
				func();
		m_SubmitFunctions.clear();

		SNG_INFO("Engine close");
	}

	void Application::Run()
	{
		while (!m_Window->IsClose())
		{
			static float time = 0.0f;
			float elapsedTime = (float)glfwGetTime();
			float deltaTime = elapsedTime - time;
			time = elapsedTime;

			for (auto& function : m_SubmitFunctions)
				function();

			m_SubmitFunctions.clear();
			for (auto& layer : m_LayerStack)
				layer->OnUpdate(deltaTime);

			VulkanContext::BeginFrame();
			Editor::BeginFrame();

			for (auto& layer : m_LayerStack)
				layer->OnImGuiRender();

			Editor::EndFrame();
			VulkanContext::EndFrame();

			m_Window->OnUpdate();
		}
	}

	void Application::SubmitFunction(const std::function<void()>& func)
	{
		m_SubmitFunctions.push_back(func);
	}
}
