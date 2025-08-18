#pragma once

#include "Renderer.hpp"
#include <memory>

class DebugDepthRenderer;
class ShadowRenderer;
class SimpleTextureRenderer;
class DepthPassRenderer;
class GBufferRenderer;
class CubemapUnfoldRenderer;
// fwd declaration

class RenderManager
{
private:
  std::shared_ptr<DebugDepthRenderer> debugDepthRenderer = nullptr;
  std::shared_ptr<ShadowRenderer> shadowRenderer = nullptr;
  std::shared_ptr<SimpleTextureRenderer> simpleTextureRenderer = nullptr;
  std::shared_ptr<DepthPassRenderer> depthPassRenderer = nullptr;
  std::shared_ptr<GBufferRenderer> gbufferRenderer = nullptr;
  std::shared_ptr<CubemapUnfoldRenderer> cubemapUnfoldRenderer = nullptr;
  std::shared_ptr<Renderer> currentRenderer = nullptr;
  void clearContext();

public:
  int rendererWidth = 1600;
  int rendererHeight = 900;

  enum Mode
  {
    point_shadow,
    parallel_shadow,
    debug_depth,
    simple_texture,
    depth_pass,
    gbuffer,
    cubemap_unfold
  };
  RenderManager();
  void reloadShadowShaders(Shader &&mainShader, Shader &&pointShadowShader);
  void reloadCurrentShaders();
  void switchMode(Mode _mode);
  void switchContext();
  void render(RenderParameters &renderParameters);
  void resize(int _width, int _height);
};