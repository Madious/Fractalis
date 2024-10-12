
-- Vulkan SDK
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Third party include directories
IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["ShaderC"] = "%{IncludeDir.VulkanSDK}/shaderc"
IncludeDir["GLFW"] = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["ImGui"] = "%{wks.location}/Dependencies/ImGui"
IncludeDir["spdlog"] = "%{wks.location}/Dependencies/spdlog/include"
IncludeDir["glm"] = "%{wks.location}/Dependencies/glm"

-- Third party .lib files
Library = {}
Library["VulkanSDK"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"
Library["ShaderC_Debug"] = "%{VULKAN_SDK}/Lib/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib"
Library["ShaderC_Release"] = "%{VULKAN_SDK}/Lib/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"