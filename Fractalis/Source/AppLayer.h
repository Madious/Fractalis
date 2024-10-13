#pragma once
#include <Core/Layer.h>
#include <Rendering/ComputeShader.h>


namespace SNG
{

	enum class Fractal
	{
		Julia, Mandelbrot
	};

	enum class Color
	{
		Cosmic, Temperature, Ocean, Fire, BlackAndWhite
	};

	struct Complex
	{
		double Re = 0.0;
		double Im = 0.0;
	};

	struct SSBO
	{
		alignas(4) Fractal Fractal = Fractal::Julia;
		alignas(4) Color Color = Color::Cosmic;
		alignas(16) Complex C;
		alignas(4) int MaxIteration = 80;
		alignas(4) float Zoom = 1.0f;
		alignas(16) glm::dvec2 Offset = Vec2(0.0);
	};

	class AppLayer : public Layer
	{
	public:

		AppLayer();

		void OnUpdate(float ts) override;
		void OnImGuiRender() override;

	private:

		SSBO m_SSBO;
		Vec2 m_ViewportSize;
		Ref<Image> m_Image;
		Ref<Buffer> m_SSBOBuffer;
		Ref<ComputeShader> m_ComputeShader;
	};
}
