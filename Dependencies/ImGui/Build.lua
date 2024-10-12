
project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"imconfig.h",
        "imgui.h",
        "imgui.cpp",
        "imgui_tables.cpp",
        "imgui_internal.h",
        "imgui_draw.cpp",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
        "imgui_demo.cpp",
        "imgui_stdlib.h",
        "imgui_stdlib.cpp",
        "imgui_impl_glfw.cpp",
		"imgui_impl_vulkan.cpp",
	}

	includedirs
	{
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
