#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <cstdarg> // va_list, va_start, va_end
#include <format>  // C++20
#include <chrono>

#include "Utils.hpp"
#include "LogFormat.hpp"

/**
 * @brief 静态调试日志输出类，支持格式化文本和可视化控制面板。
 * @note 所有方法均为静态调用，无需实例化。
 */
class DebugOutput
{
public:
    /**
     * @brief 添加一条格式化日志到输出缓冲区
     * @tparam Args 可变参数类型。
     * @param fmt 格式化字符串，遵循 std::format 语法（C++20）。
     * @param args 要格式化的参数。
     */
    template <typename... Args>
    static void AddLog(std::format_string<Args...> fmt, Args &&...args);

    static void AddLog(std::string string);

    /**
     * @brief 清空所有日志记录。
     */
    static void Clear();

    /**
     * @brief 绘制调试输出窗口。
     * @param title 窗口标题，默认为 "Debug Output"。
     * @param p_open 可选的指向窗口显示状态的指针。
     */
    static void Draw(const char *title = "Debug Output", bool *p_open = nullptr);

    static void ExportShaderSource(const std::string &filename, const std::string &source)
    {
        // 尝试创建父目录。如果失败，立即返回。
        if (!Utils::CreateParentDirectories(filename))
        {
            std::cerr << "Cannot export shader source due to directory creation failure." << std::endl;
            return;
        }
        std::ofstream file(filename);
        if (file.is_open())
        {
            file << source;
            file.close();
            std::cerr << "Shader source exported to " << filename << std::endl;
        }
        else
        {
            std::cerr << "Failed to export shader source to " << filename << std::endl;
        }
    }

private:
    inline static std::vector<std::string> s_logs;
    inline static bool s_autoScroll = true;
};

// 静态成员函数的内联实现
// (inline 关键字对于在类体内定义的函数是隐式的，但在类体外定义时，需要显式添加)

template <typename... Args>
inline void DebugOutput::AddLog(std::format_string<Args...> fmt, Args &&...args)
{
    s_logs.emplace_back(std::format(fmt, std::forward<Args>(args)...));
}

inline void DebugOutput::AddLog(std::string string)
{
    s_logs.emplace_back(string);
}

inline void DebugOutput::Clear()
{
    s_logs.clear();
}

inline void DebugOutput::Draw(const char *title, bool *p_open)
{
    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        Clear();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &s_autoScroll);

    ImGui::Separator();
    ImGui::BeginChild(
        "ScrollingRegion",
        ImVec2(0, 0),
        false,
        ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto &log : s_logs)
    {
        RenderColoredText(log);
    }

    if (s_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}