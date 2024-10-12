
workspace "Fractalis Engine"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	flags { "MultiProcessorCompile" }
	startproject "Fractalis"



-- Output directory
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


-- Dependencies
group "Dependencies"
	include "Dependencies/GLFW/Build.lua"
	include "Dependencies/ImGui/Build.lua"
group ""

include "External.lua"
include "Synergy/Build.lua"
include "Fractalis/Build.lua"
