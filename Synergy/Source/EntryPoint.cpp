
#include "Snpch.h"
#include "Core/Application.h"


extern SNG::Application* SNG::CreateApplication();


#ifdef SNG_DIST

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	SNG::Application* app = SNG::CreateApplication();
	app->Run();
	delete app;
}

#else

int main()
{
	SNG::Application* app = SNG::CreateApplication();
	app->Run();
	delete app;
}

#endif
