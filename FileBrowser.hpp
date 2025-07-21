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

class FileSelector
{
public:
    FileSelector() = default;

    // 显示文件选择器窗口
    void Show()
    {
        // 显示当前选择的文件路径
        ImGui::Text("Selected Files (%zu):", m_selectedPaths.size());
        ImGui::BeginChild("SelectedFiles", ImVec2(0, 150), true);
        for (const auto &path : m_selectedPaths)
        {
            ImGui::Text("%s", path.c_str());
        }
        ImGui::EndChild();

        // 选择文件按钮
        if (ImGui::Button("Select Files"))
        {
            auto paths = OpenSystemFileDialog(true);
            if (!paths.empty())
            {
                m_selectedPaths.insert(m_selectedPaths.end(), paths.begin(), paths.end());
                m_currentPaths = std::move(paths);
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
    const std::vector<std::string> &GetCurrentPaths() const
    {
        return m_currentPaths;
    }

    // 获取所有选择的文件路径
    const std::vector<std::string> &GetAllPaths() const
    {
        return m_selectedPaths;
    }

    // 清除所有选择的路径
    void ClearPaths()
    {
        m_selectedPaths.clear();
        m_currentPaths.clear();
    }

private:
    std::vector<std::string> m_selectedPaths;
    std::vector<std::string> m_currentPaths;

    // 打开系统文件对话框 (支持多选)
    std::vector<std::string> OpenSystemFileDialog(bool multiSelect)
    {
        std::vector<std::string> result;

#ifdef _WIN32
        OPENFILENAMEA ofn;
        CHAR szFile[1024 * 16] = {0};

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
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // Prevent System Change Working Directory

        if (multiSelect)
        {
            ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
        }

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            char *p = szFile;
            std::string directory = p;
            p += directory.size() + 1;

            if (*p)
            {
                while (*p)
                {
                    std::string filename = p;
                    result.push_back(directory + "\\" + filename);
                    p += filename.size() + 1;
                }
            }
            else
            {
                result.push_back(directory);
            }
        }
#else
        const char *cmd = nullptr;
        if (access("/usr/bin/zenity", X_OK) == 0)
        {
            cmd = multiSelect ? "zenity --file-selection --multiple --separator='\n'"
                              : "zenity --file-selection";
        }
        else if (access("/usr/bin/kdialog", X_OK) == 0)
        {
            cmd = multiSelect ? "kdialog --getopenfilename --multiple"
                              : "kdialog --getopenfilename";
        }

        if (cmd)
        {
            FILE *f = popen(cmd, "r");
            if (f)
            {
                char path[1024];
                while (fgets(path, sizeof(path), f))
                {
                    size_t len = strlen(path);
                    if (len > 0 && path[len - 1] == '\n')
                    {
                        path[len - 1] = '\0';
                    }
                    if (path[0] != '\0')
                    {
                        result.push_back(path);
                    }
                }
                pclose(f);
            }
        }
#endif

        // 移除重复路径
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
};