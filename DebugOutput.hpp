#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <cstdarg> // va_list, va_start, va_end
#include <format>  // C++20 或 #include <fmt/core.h>
#include <chrono>

/**
 * @brief 获取当前时间的格式化字符串或时间戳
 * @param fmt 格式字符串（默认为 "%Y-%m-%d %H:%M:%S"）
 *            特殊值 "unix" 返回 Unix 时间戳（秒）
 *            特殊值 "unix_ms" 返回毫秒级时间戳
 * @return 格式化后的时间字符串或时间戳字符串
 **/
std::string FormatTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *local_time = std::localtime(&now); // 转换为本地时间结构体

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
     * @brief 添加一条格式化日志到输出缓冲区 example: DebugOutput::AddLog("Player {} took {:.1f} damage", "Alice", 12.345);
     * @tparam Args 可变参数类型（自动推导）。
     * @param fmt 格式化字符串，遵循 std::format 语法（C++20）或 fmt 库语法。
     * @param args 要格式化的参数，类型必须与 fmt 中的占位符匹配。
     *
     * @example
     * DebugOutput::AddLog("Player {} took {:.1f} damage", "Alice", 12.345);
     */
    template <typename... Args>
    static void AddLog(std::format_string<Args...> fmt, Args &&...args);

    static void AddLog(std::string string);

    /**
     * @brief 清空所有日志记录。
     */
    static void Clear();

    /**
     *
     * @brief 绘制调试输出窗口。
     * @param title 窗口标题（显示在左上角），默认为 "Debug Output"。
     * @param p_open 可选的指向窗口显示状态的指针。若不为 nullptr，窗口将显示关闭按钮，并通过该指针返回窗口是否可见。
     *
     * @note 此函数必须在 ImGui 帧渲染循环中调用（介于 NewFrame() 和 Render() 之间）。
     * @example
     * static bool show_debug = true;
     * DebugOutput::Draw("My Logs", &show_debug);
     */
    static void Draw(const char *title = "Debug Output", bool *p_open = nullptr);

private:
    static std::vector<std::string> s_logs; // 日志存储缓冲区
    static bool s_autoScroll;               // 自动滚动标志位
};
// 静态成员初始化
std::vector<std::string> DebugOutput::s_logs;
bool DebugOutput::s_autoScroll = true;

/**
 * @brief 添加一条格式化日志到输出缓冲区。
 * @tparam Args 可变参数类型（自动推导）。
 * @param fmt 格式化字符串，遵循 std::format 语法（C++20）或 fmt 库语法。
 * @param args 要格式化的参数，类型必须与 fmt 中的占位符匹配。
 *
 * @example DebugOutput::AddLog("Player {} took {:.1f} damage", "Alice", 12.345);
 *
 */
template <typename... Args>
void DebugOutput::AddLog(std::format_string<Args...> fmt, Args &&...args)
{
    s_logs.emplace_back(std::format(fmt, std::forward<Args>(args)...));
}

void DebugOutput::AddLog(std::string string)
{
    s_logs.emplace_back(string);
}

void DebugOutput::Clear()
{
    s_logs.clear(); // 清空所有日志（内存保留，避免重复分配）
}

void DebugOutput::Draw(const char *title, bool *p_open)
{
    // 开始窗口（遵循 ImGui 的 Begin/End 模式）
    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End(); // 提前退出时需手动调用 End()
        return;
    }

    // ---- 控制按钮区域 ----
    // [Clear] 按钮：触发日志清空
    if (ImGui::Button("Clear"))
    {
        Clear(); // 实际调用 s_logs.clear()
    }

    // [Auto-scroll] 复选框：绑定到 s_autoScroll 标志位
    ImGui::SameLine(); // 与上一个控件同行
    ImGui::Checkbox("Auto-scroll", &s_autoScroll);

    // ---- 日志显示区域 ----
    ImGui::Separator(); // 水平分隔线
    ImGui::BeginChild(
        "ScrollingRegion",                   // 子区域 ID
        ImVec2(0, 0),                        // 自动填充父窗口剩余空间
        false,                               // 不显示边框
        ImGuiWindowFlags_HorizontalScrollbar // 启用水平滚动条
    );

    // 遍历所有日志条目
    for (const auto &log : s_logs)
    {
        ImGui::TextUnformatted(log.c_str()); // 直接输出原始文本（无额外解析）
    }

    // 自动滚动逻辑：当标志为真且用户未手动滚动时，强制滚动到底部
    if (s_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f); // 1.0f 表示底部对齐
    }

    ImGui::EndChild(); // 结束子区域
    ImGui::End();      // 结束窗口
}