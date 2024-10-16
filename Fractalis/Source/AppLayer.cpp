#include "AppLayer.h"
#include <Input/Input.h>
#include <Core/Application.h>


namespace SNG
{

	AppLayer::AppLayer()
	{
		m_ViewportSize = { 1600.0f, 900.0f };
		m_Image = MakeRef<Image>(m_ViewportSize.x, m_ViewportSize.y);
		m_SSBOBuffer = MakeRef<Buffer>(sizeof(SSBO));

		m_ComputeShader = MakeRef<ComputeShader>("Resources/Shaders/Fractal.glsl");
		m_ComputeShader->AddUniformImage(m_Image, ImageLayout::Shader);
		m_ComputeShader->AddUniformBuffer(m_SSBOBuffer);
		m_ComputeShader->FinalizeUniformCreation();
	}

	void AppLayer::OnUpdate(float ts)
	{
		if (Input::IsKeyPress(GLFW_KEY_KP_ADD))
			m_SSBO.Zoom -= m_SSBO.Zoom * ts * m_ZoomSpeed;
		if (Input::IsKeyPress(GLFW_KEY_KP_SUBTRACT))
			m_SSBO.Zoom += m_SSBO.Zoom * ts * m_ZoomSpeed;
		if (Input::IsKeyPress(GLFW_KEY_UP))
			m_SSBO.Offset.y -= m_TranslationSpeed * m_SSBO.Zoom * ts;
		if (Input::IsKeyPress(GLFW_KEY_DOWN))
			m_SSBO.Offset.y += m_TranslationSpeed * m_SSBO.Zoom * ts;
		if (Input::IsKeyPress(GLFW_KEY_LEFT))
			m_SSBO.Offset.x -= m_TranslationSpeed * m_SSBO.Zoom * ts;
		if (Input::IsKeyPress(GLFW_KEY_RIGHT))
			m_SSBO.Offset.x += m_TranslationSpeed * m_SSBO.Zoom * ts;
	}

	void AppLayer::OnImGuiRender()
	{
		//===== SETTINGS
		ImGui::Begin("Settings");

		// Fractal choice
		const char* fractals[2] = { "Julia", "Mandelbrot" };
		if (ImGui::BeginCombo("Fractal", fractals[(int)m_SSBO.Fractal]))
		{
			for (int i = 0; i < 2; i++)
			{
				bool selected = (int)m_SSBO.Fractal == i;
				if (ImGui::Selectable(fractals[i], &selected))
					m_SSBO.Fractal = (Fractal)i;
			}

			ImGui::EndCombo();
		}

		// Color choice
		const char* colors[5] = { "Cosmic", "Temperature", "Ocean", "Fire", "Black and white" };
		if (ImGui::BeginCombo("Color", colors[(int)m_SSBO.Color]))
		{
			for (int i = 0; i < 5; i++)
			{
				bool selected = (int)m_SSBO.Color == i;
				if (ImGui::Selectable(colors[i], &selected))
					m_SSBO.Color = (Color)i;
			}

			ImGui::EndCombo();
		}

		// Constant for Julia
		if (m_SSBO.Fractal == Fractal::Julia)
		{
			ImGui::DragScalar("Re", ImGuiDataType_Double, &m_SSBO.C.Re, 0.001f);
			ImGui::DragScalar("Im", ImGuiDataType_Double, &m_SSBO.C.Im, 0.001f);
		}

		ImGui::DragInt("Max Iteration", &m_SSBO.MaxIteration, 1.0f, 0, INT_MAX);
		ImGui::SliderFloat("Zoom Speed", &m_ZoomSpeed, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Translation Speed", &m_TranslationSpeed, 0.0f, 10.0f, "%.1f");
		ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::End();


		//===== VIEWPORT
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = Vec2{ viewportSize.x, viewportSize.y };

		m_SSBOBuffer->SetData(&m_SSBO, sizeof(SSBO));
		m_Image->Resize(m_ViewportSize.x, m_ViewportSize.y);
		uint32_t dispatchX = (m_Image->GetWidth() + 16 - 1) / 16;
		uint32_t dispatchY = (m_Image->GetHeight() + 16 - 1) / 16;
		m_ComputeShader->Run(dispatchX, dispatchY, 1);

		ImGui::Image((ImTextureID)m_Image->GetDescriptorSet(), viewportSize);
		ImGui::End();
		ImGui::PopStyleVar();
	}
}
