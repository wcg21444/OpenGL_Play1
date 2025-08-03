#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <cstdarg> // va_list, va_start, va_end
#include <format>  // C++20
#include <chrono>

/**
 * @brief 获取当前时间的格式化字符串
 * @return 格式化后的时间字符串
 **/
inline std::string FormatTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *local_time = std::localtime(&now);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);

    return std::string(buffer);
}

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

private:
    // C++17 或更高版本中，使用 inline 关键字允许在头文件中定义静态成员变量。
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
        ImGui::TextUnformatted(log.c_str());
    }

    if (s_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}