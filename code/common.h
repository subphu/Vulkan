//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "helper.h"

#define PI 3.14159265358979323846

#ifdef NDEBUG
#define IS_DEBUG false
#else
#define IS_DEBUG true
#endif

#define CHECK_BOOL(r,m) if(!r)std::runtime_error(m)
#define CHECK_VKRESULT(r,m) if(r!=VK_SUCCESS)std::runtime_error(m)
#define CHECK_HANDLE(r,m) if(r==VK_NULL_HANDLE)std::runtime_error(m)

#define PRINT(v1) std::cout << v1
#define PRINT(v1, v2) PRINT(v1) << " " << v2
#define PRINT(v1, v2, v3) PRINT(v1, v2) << " " << v3
#define PRINT(v1, v2, v3, v4) PRINT(v1, v2, v3) << " " << v4
#define PRINTLN(v1) PRINT(v1) << std::endl
#define PRINTLN(v1, v2) PRINT(v1, v2) << std::endl
#define PRINTLN(v1, v2, v3) PRINT(v1, v2, v3) << std::endl
#define PRINTLN(v1, v2, v3, v4) PRINT(v1, v2, v3, v4) << std::endl


