//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <math.h>

#pragma clang diagnostic pop

#define PI 3.14159265358979323846

#ifdef NDEBUG
#define IS_DEBUG false
#else
#define IS_DEBUG true
#endif

#define USE_VAR(v) {}
#define USE_FUNC(f) {}

#define RUNTIME_ERROR(m) throw std::runtime_error(m)

#define CHECK_BOOL(v,m) if(!v) RUNTIME_ERROR(m)
#define CHECK_ZERO(v,m) if(!v) RUNTIME_ERROR(m)
#define CHECK_MINUS(v,m) if(v<0) RUNTIME_ERROR(m)
#define CHECK_HANDLE(v,m) if(v==VK_NULL_HANDLE) RUNTIME_ERROR(m)

#define CHECK_OBJECT(v,m) if(v==nullptr) RUNTIME_ERROR(m)
#define CHECK_POINTER(v,m) if(v==nullptr) RUNTIME_ERROR(m)

#define CHECK_VKRESULT(r,m) if(r!=VK_SUCCESS) RUNTIME_ERROR(m)


#define LOG(v) std::cout << "LOG::" << v << std::endl
#define PRINT1(  v1            ) std::cout << v1
#define PRINT2(  v1, v2        ) PRINT1(v1        ) << " " << v2
#define PRINT3(  v1, v2, v3    ) PRINT2(v1, v2    ) << " " << v3
#define PRINT4(  v1, v2, v3, v4) PRINT3(v1, v2, v3) << " " << v4
#define PRINTLN1(v1            ) PRINT1(v1            ) << std::endl
#define PRINTLN2(v1, v2        ) PRINT2(v1, v2        ) << std::endl
#define PRINTLN3(v1, v2, v3    ) PRINT3(v1, v2, v3    ) << std::endl
#define PRINTLN4(v1, v2, v3, v4) PRINT4(v1, v2, v3, v4) << std::endl

#define UINT32(v) static_cast<uint32_t>(v)

template<typename T> struct Size { T width, height; };

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float, std::chrono::seconds::period> TimeDif;
