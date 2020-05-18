#include "aepch.h"
#include "Renderer2D.h"

#include "Ancora/Renderer/VertexArray.h"
#include "Ancora/Renderer/Shader.h"
#include "Ancora/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Ancora {

  struct QuadVertex
  {
    glm::vec4 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;
    float TexIndex;
    float TilingFactor;
  };

  struct Renderer2DStorage
  {
    const uint32_t MaxQuads = 10000;
    const uint32_t MaxVertices = MaxQuads * 4;
    const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 32;

    Ref<VertexArray> QuadVertexArray;
    Ref<VertexBuffer> QuadVertexBuffer;
    Ref<Shader> TextureShader;
    Ref<Texture2D> WhiteTexture;

    uint32_t QuadIndexCount = 0;
    QuadVertex* QuadVertexBufferBase = nullptr;
    QuadVertex* QuadVertexBufferPtr = nullptr;

    std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1;
  };

  static Renderer2DStorage s_Data;

  void Renderer2D::Init()
  {
    s_Data.QuadVertexArray = VertexArray::Create();

    s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
    BufferLayout layout = {
      { ShaderDataType::Float4, "a_Position" },
      { ShaderDataType::Float4, "a_Color" },
      { ShaderDataType::Float2, "a_TexCoord" },
      { ShaderDataType::Float,  "a_TexIndex" },
      { ShaderDataType::Float,  "a_TilingFactor" }
    };
    s_Data.QuadVertexBuffer->SetLayout(layout);
    s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

    s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

    uint32_t* indices = new uint32_t[s_Data.MaxIndices];

    uint32_t offset = 0;
    for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
    {
      indices[i + 0] = offset + 0;
      indices[i + 1] = offset + 1;
      indices[i + 2] = offset + 2;

      indices[i + 3] = offset + 2;
      indices[i + 4] = offset + 3;
      indices[i + 5] = offset + 0;

      offset += 4;
    }

    Ref<IndexBuffer> indexBuffer;
    indexBuffer = IndexBuffer::Create(indices, s_Data.MaxIndices);
    s_Data.QuadVertexArray->SetIndexBuffer(indexBuffer);
    delete[] indices;

    s_Data.WhiteTexture = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    int32_t samplers[s_Data.MaxTextureSlots];
    for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
      samplers[i] = i;

    s_Data.TextureShader = Shader::Create("Sandbox/assets/shaders/Texture.glsl");
    s_Data.TextureShader->Bind();
    s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

    s_Data.TextureSlots[0] = s_Data.WhiteTexture;
  }

  void Renderer2D::Shutdown()
  {
  }

  void Renderer2D::BeginScene(const OrthographicCamera& camera)
  {
    s_Data.TextureShader->Bind();
    s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

    s_Data.TextureSlotIndex = 1;
  }

  void Renderer2D::EndScene()
  {
    uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
    s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

    Flush();
  }

  void Renderer2D::Flush()
  {
    for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
    {
      s_Data.TextureSlots[i]->Bind(i);
    }

    RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
  }

  void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
  {
    DrawQuad({ position.x, position.y, 0.0f }, size, color);
  }

  void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
  {
    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    // s_Data.TextureShader->SetInt("u_TilingFactor", 1);
    // s_Data.WhiteTexture->Bind(0);
    //
    // glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));
    // s_Data.TextureShader->SetMat4("u_Transform", transform);
    //
    // RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
  }

  void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor, const glm::vec4& tintColor)
  {
    DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
  }

  void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor, const glm::vec4& tintColor)
  {
    constexpr glm::vec2 textureCoords[] = {
      { 0.0f, 0.0f },
      { 1.0f, 0.0f },
      { 1.0f, 1.0f },
      { 0.0f, 1.0f }
    };

    float textureIndex = 0.0f;

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
      if (*s_Data.TextureSlots[i].get() == *texture.get())
      {
        textureIndex = (float)i;
        break;
      }
    }

    if (textureIndex == 0.0f)
    {
      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
    }

    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[0];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[1];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[2];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[3];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    // s_Data.TextureShader->SetInt("u_TilingFactor", tilingFactor);
    // s_Data.TextureShader->SetFloat4("u_Color", color);
    // texture->Bind();
    //
    // glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));
    // s_Data.TextureShader->SetMat4("u_Transform", transform);
    //
    // s_Data.QuadVertexArray->Bind();
    // RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
  }

  void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor, const glm::vec4& tintColor)
  {
    DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, tilingFactor, tintColor);
  }

  void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor, const glm::vec4& tintColor)
  {
    const glm::vec2* textureCoords = subtexture->GetTexCoords();
    const Ref<Texture2D> texture = subtexture->GetTexture();

    float textureIndex = 0.0f;

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
      if (*s_Data.TextureSlots[i].get() == *texture.get())
      {
        textureIndex = (float)i;
        break;
      }
    }

    if (textureIndex == 0.0f)
    {
      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
    }

    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[0];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[1];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[2];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = { position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f };
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[3];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    // s_Data.TextureShader->SetInt("u_TilingFactor", tilingFactor);
    // s_Data.TextureShader->SetFloat4("u_Color", color);
    // texture->Bind();
    //
    // glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));
    // s_Data.TextureShader->SetMat4("u_Transform", transform);
    //
    // s_Data.QuadVertexArray->Bind();
    // RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color)
  {
    DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, color);
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color)
  {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
      * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
      * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
    s_Data.QuadVertexBufferPtr->TexIndex = 0.0f;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    // s_Data.TextureShader->SetInt("u_TilingFactor", 1);
    // s_Data.TextureShader->SetFloat4("u_Color", color);
    // s_Data.WhiteTexture->Bind();
    //
    //
    // s_Data.TextureShader->SetMat4("u_Transform", transform);
    //
    // RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor, const glm::vec4& tintColor)
  {
    DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, texture, tilingFactor, tintColor);
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, int tilingFactor, const glm::vec4& tintColor)
  {
    constexpr glm::vec2 textureCoords[] = {
      { 0.0f, 0.0f },
      { 1.0f, 0.0f },
      { 1.0f, 1.0f },
      { 0.0f, 1.0f }
    };

    float textureIndex = 0.0f;

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
      if (*s_Data.TextureSlots[i].get() == *texture.get())
      {
        textureIndex = (float)i;
        break;
      }
    }

    if (textureIndex == 0.0f)
    {
      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
      * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
      * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[0];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[1];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[2];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[3];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor, const glm::vec4& tintColor)
  {
    DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, subtexture, tilingFactor, tintColor);
  }

  void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, int tilingFactor, const glm::vec4& tintColor)
  {
    const glm::vec2* textureCoords = subtexture->GetTexCoords();
    const Ref<Texture2D> texture = subtexture->GetTexture();

    float textureIndex = 0.0f;

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
      if (*s_Data.TextureSlots[i].get() == *texture.get())
      {
        textureIndex = (float)i;
        break;
      }
    }

    if (textureIndex == 0.0f)
    {
      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
      * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
      * glm::scale(glm::mat4(1.0f), glm::vec3({ size.x, size.y, 1.0f }));

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[0];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, -0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[1];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ 0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[2];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4({ -0.5f, 0.5, 0.0f, 1.0f });
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[3];
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;
  }

}
