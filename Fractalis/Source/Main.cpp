#include <Core/Application.h>
#include "AppLayer.h"


SNG::Application* SNG::CreateApplication()
{
	auto app = new Application({ 1600.0f, 900.0f }, "Fractalis");
	app->PushLayer<AppLayer>();
	return app;
}
