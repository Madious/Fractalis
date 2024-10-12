
#pragma once

/* THIRD PARTY */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_stdlib.h>


/* STANDART LIBRARY */
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <utility>
#include <thread>
#include <execution>
#include <random>
