
#include "Snpch.h"
#include "Core/Application.h"


extern SNG::Application* SNG::CreateApplication();


int main()
{
	SNG::Application* app = SNG::CreateApplication();
	app->Run();
	delete app;
}