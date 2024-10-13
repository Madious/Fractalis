
project "Fractalis"
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
		kind "ConsoleApp"
		defines "SNG_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		kind "ConsoleApp"
		defines "SNG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "SNG_DIST"
		runtime "Release"
		optimize "on"
