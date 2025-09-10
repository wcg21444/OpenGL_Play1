#pragma once

#include <iostream>
#include <functional>
#include <queue>
#include <memory>
class DebugObjectPass;
class Camera;
using DebugObjectDrawCall = std::function<void(Shader &shaders, glm::mat4 modelMatrix)>;

// 辅助对象渲染器. 单独使用一条渲染管线
// 不应该存储任何渲染参数状态,所有状态都应该在调用时传入
class DebugObjectRenderer
{
private:
    inline static std::queue<DebugObjectDrawCall> drawQueue;
    inline static int width = 1600;
    inline static int height = 900;
    inline static std::shared_ptr<DebugObjectPass> debugObjectPass;

public:
    static void AddDrawCall(const DebugObjectDrawCall &drawCall);
    static void Initialize();
    static void Resize(int _width, int _height);
    static void Render(Camera& camera);
    static void ReloadCurrentShaders();
    static unsigned int GetRenderOutput();
    static void CheckInitialized();

};
