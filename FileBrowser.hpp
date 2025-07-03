// ImGuiMultiFileSelector.hpp
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

class ImGuiMultiFileSelector
{
public:
    // 初始化文件选择器
    inline static void Initialize()
    {
        s_SelectedPaths.clear();
        s_CurrentPaths.clear();
    }

    // 显示文件选择器窗口
    inline static void Show()
    {

        // 显示当前选择的文件路径
        ImGui::Text("Selected Files (%zu):", s_SelectedPaths.size());
        ImGui::BeginChild("SelectedFiles", ImVec2(0, 150), true);
        for (const auto &path : s_SelectedPaths)
        {
            ImGui::Text("%s", path.c_str());
        }
        ImGui::EndChild();

        // 选择文件按钮
        if (ImGui::Button("Select Files"))
        {
            s_CurrentPaths = OpenSystemFileDialog(true);
            if (!s_CurrentPaths.empty())
            {
                s_SelectedPaths.insert(s_SelectedPaths.end(), s_CurrentPaths.begin(), s_CurrentPaths.end());
            }
        }

        ImGui::SameLine();

        // 清除按钮
        if (ImGui::Button("Clear All"))
        {
            ClearPaths();
        }
    }

    // 获取最新选择的文件路径列表
    inline static const std::vector<std::string> &GetCurrentPaths()
    {
        return s_CurrentPaths;
    }

    // 获取所有选择的文件路径
    inline static const std::vector<std::string> &GetAllPaths()
    {
        return s_SelectedPaths;
    }

    // 清除所有选择的路径
    inline static void ClearPaths()
    {
        s_SelectedPaths.clear();
        s_CurrentPaths.clear();
    }

private:
    inline static std::vector<std::string> s_SelectedPaths;
    inline static std::vector<std::string> s_CurrentPaths;

    // 打开系统文件对话框 (支持多选)
    inline static std::vector<std::string> OpenSystemFileDialog(bool multiSelect)
    {
        std::vector<std::string> result;

#ifdef _WIN32
        OPENFILENAMEA ofn;
        CHAR szFile[1024 * 16] = {0}; // 足够大的缓冲区存储多个文件路径

        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (multiSelect)
        {
            ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
        }

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            // 处理多选结果
            char *p = szFile;
            std::string directory = p;
            p += directory.size() + 1;

            if (*p)
            { // 多选情况
                while (*p)
                {
                    std::string filename = p;
                    result.push_back(directory + "\\" + filename);
                    p += filename.size() + 1;
                }
            }
            else
            { // 单选情况
                result.push_back(directory);
            }
        }
#else
        // Linux/macOS 使用 zenity 或 kdialog
        const char *cmd = nullptr;

        if (access("/usr/bin/zenity", X_OK) == 0)
        {
            cmd = multiSelect ? "zenity --file-selection --multiple" : "zenity --file-selection";
        }
        else if (access("/usr/bin/kdialog", X_OK) == 0)
        {
            cmd = multiSelect ? "kdialog --getopenfilename --multiple" : "kdialog --getopenfilename";
        }

        if (cmd)
        {
            FILE *f = popen(cmd, "r");
            if (f)
            {
                char path[1024];
                while (fgets(path, sizeof(path), f))
                {
                    // 移除末尾的换行符
                    size_t len = strlen(path);
                    if (len > 0 && path[len - 1] == '\n')
                    {
                        path[len - 1] = '\0';
                    }
                    result.push_back(path);
                }
                pclose(f);
            }
        }
#endif
        return result;
    }
};