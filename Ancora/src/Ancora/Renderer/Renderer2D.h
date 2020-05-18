#pragma once

#include "OrthographicCamera.h"

#include "Texture.h"
#include "SubTexture2D.h"

namespace Ancora {

  class Renderer2D
  {
  public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const OrthographicCamera& camera);
    static void EndScene();
    static void Flush();

    // Primitives
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));

    static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color);
    static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color);
    static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor = 1, const glm::vec4& tintColor = glm::vec4(1.0f));
  };

}
