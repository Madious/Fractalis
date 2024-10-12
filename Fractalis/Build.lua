
project "Fractalis"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Source/**.h",
		"Source/**.c",
		"Source/**.hpp",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",
		"%{wks.location}/Synergy/Source",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
	}

	links
	{
		"Synergy"
	}

	filter "system:windows"
		systemversion "latest"
		defines "SNG_PLATFORM_WINDOWS"

	filter "configurations:Debug"
		defines "SNG_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SNG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SNG_DIST"
		runtime "Release"
		optimize "on"
