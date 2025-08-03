cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(OpenGL_Play)
set(CMAKE_CXX_STANDARD 20)


set(IMGUI 
imgui
imgui/backends
)
set (INCLUDES includes)


file(GLOB_RECURSE IMGUI_SRC 
    imgui/*.cpp
)

# set(RENDERERS_SRC
#     Renderers/GBufferRenderer.cpp
# )

include_directories(${IMGUI})
include_directories("Objects")
include_directories(${INCLUDES})

file(GLOB OBJECTS_SRC
    Objects/*.cpp
)
add_executable(${PROJECT_NAME} "main.cpp" ${IMGUI_SRC} ${RENDERERS_SRC} ${OBJECTS_SRC})

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(OpenGL REQUIRED) # windows SDK中有opengl的库
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)