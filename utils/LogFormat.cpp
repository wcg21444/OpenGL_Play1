#include "LogFormat.hpp"
#include <vector>
#include <format>
#include <chrono>

// 预定义的颜色标签
const std::vector<ColorTag> COLOR_TAGS = {
    {"<error>", ImVec4(1.0f, 0.0f, 0.0f, 1.0f)},     // 红色错误
    {"<warning>", ImVec4(1.0f, 1.0f, 0.0f, 1.0f)},   // 黄色警告
    {"<info>", ImVec4(0.0f, 1.0f, 0.0f, 1.0f)},      // 绿色信息
    {"<highlight>", ImVec4(0.5f, 0.5f, 1.0f, 1.0f)}, // 蓝色高亮
    {"<path>", ImVec4(0.7f, 0.7f, 0.7f, 1.0f)}};

// color格式规则 :  <Tag>有颜色文字</Tag>
void RenderColoredText(const std::string &text)
{
    size_t current_pos = 0;
    size_t tag_start = 0;
    size_t tag_end = 0;
    ImVec4 current_color = ImGui::GetStyle().Colors[ImGuiCol_Text]; // 默认颜色

    while (current_pos < text.length())
    {
        // 查找开始标记
        tag_start = text.find('<', current_pos);
        if (tag_start == std::string::npos)
        {
            // 没有更多标记，输出剩余文本
            ImGui::TextUnformatted(text.substr(current_pos).c_str());
            break;
        }

        // 输出标记前的普通文本
        if (tag_start > current_pos)
        {
            ImGui::TextUnformatted(text.substr(current_pos, tag_start - current_pos).c_str());
            ImGui::SameLine();
        }

        // 查找结束标记
        tag_end = text.find('>', tag_start);
        if (tag_end == std::string::npos)
        {
            // 标记不完整，输出剩余文本
            ImGui::TextUnformatted(text.substr(tag_start).c_str());
            break;
        }

        // 提取完整标记
        std::string full_tag = text.substr(tag_start, tag_end - tag_start + 1);

        // 检查是否是结束标记
        if (full_tag.find("</") == 0)
        {
            // 结束标记，恢复默认颜色
            current_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
            current_pos = tag_end + 1;
            continue;
        }

        // 查找匹配的颜色标记
        bool tag_found = false;
        for (const auto &color_tag : COLOR_TAGS)
        {
            if (full_tag == color_tag.tag)
            {
                current_color = color_tag.color;
                tag_found = true;
                break;
            }
        }

        if (!tag_found)
        {
            // 未知标记，作为普通文本输出
            ImGui::TextUnformatted(full_tag.c_str());
            ImGui::SameLine();
            current_pos = tag_end + 1;
            continue;
        }

        // 查找标记内容的结束位置
        size_t content_start = tag_end + 1;
        size_t content_end = text.find("</" + full_tag.substr(1), content_start);
        if (content_end == std::string::npos)
        {
            // 没有对应的结束标记，输出剩余文本
            ImGui::PushStyleColor(ImGuiCol_Text, current_color);
            ImGui::TextUnformatted(text.substr(content_start).c_str());
            ImGui::PopStyleColor();
            break;
        }

        // 输出带颜色的内容
        ImGui::PushStyleColor(ImGuiCol_Text, current_color);
        ImGui::TextUnformatted(text.substr(content_start, content_end - content_start).c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();

        current_pos = content_end + full_tag.length() + 1; // 跳过结束标记
    }
}

/**
 * @brief 获取当前时间的格式化字符串
 * @return 格式化后的时间字符串
 **/
std::string FormatTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *local_time = std::localtime(&now);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);

    return std::string(buffer);
}
