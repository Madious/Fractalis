
#pragma once

namespace SNG
{

	class Layer
	{
	public:

		virtual ~Layer() { }
		virtual void OnUpdate(float ts) { }
		virtual void OnImGuiRender() { }
	};
}
