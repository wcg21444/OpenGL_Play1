
#pragma once
#include <string>
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

struct ColorTag
{
    std::string tag;
    ImVec4 color;
};
void RenderColoredText(const std::string &text);

std::string FormatTime();