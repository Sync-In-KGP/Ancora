#include <Ancora.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Ancora::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_VertexArray.reset(Ancora::VertexArray::Create());

  	float vertices[] = {
			-0.75f, -0.75f,  0.0f, 0.0f, 0.0f,
			 0.75f, -0.75f,  0.0f, 1.0f, 0.0f,
			 0.75f,  0.75f,  0.0f, 1.0f, 1.0f,
			-0.75f,  0.75f,  0.0f, 0.0f, 1.0f
		};

		Ancora::Ref<Ancora::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Ancora::VertexBuffer::Create(vertices, sizeof(vertices)));
		Ancora::BufferLayout layout = {
			{ Ancora::ShaderDataType::Float3, "a_Position" },
			{ Ancora::ShaderDataType::Float2, "a_TexCoord" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[6] = {
			0, 1, 2,
			2, 3, 0
		};

		Ancora::Ref<Ancora::IndexBuffer> indexBuffer;
		indexBuffer.reset(Ancora::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_TriangleVA.reset(Ancora::VertexArray::Create());

		float triangleVertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};

		Ancora::Ref<Ancora::VertexBuffer> triangleVB;
		triangleVB.reset(Ancora::VertexBuffer::Create(triangleVertices, sizeof(triangleVertices)));
		triangleVB->SetLayout({
			{ Ancora::ShaderDataType::Float3, "a_Position" }
		});
		m_TriangleVA->AddVertexBuffer(triangleVB);

		uint32_t triangleIndices[3] = { 0, 1, 2 };
		Ancora::Ref<Ancora::IndexBuffer> triangleIB;
		triangleIB.reset(Ancora::IndexBuffer::Create(triangleIndices, sizeof(triangleIndices) / sizeof(uint32_t)));
		m_TriangleVA->SetIndexBuffer(triangleIB);

		std::string vertexSrc = R"(
			#version 450 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 450 core

			layout(location = 0) out vec4 color;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_Shader.reset(Ancora::Shader::Create(vertexSrc, fragmentSrc));

		m_TextureShader.reset(Ancora::Shader::Create("Sandbox/assets/shaders/Texture.glsl"));

		std::string blackVertexSrc = R"(
			#version 450 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string blackFragmentSrc = R"(
			#version 450 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.15, 0.15, 0.15, 1.0);
			}
		)";

		m_BlackShader.reset(Ancora::Shader::Create(blackVertexSrc, blackFragmentSrc));

		m_Texture = Ancora::Texture2D::Create("Sandbox/assets/textures/pic.png");
		m_CloudTexture = Ancora::Texture2D::Create("Sandbox/assets/textures/cloud.png");

		std::dynamic_pointer_cast<Ancora::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Ancora::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Ancora::Timestep ts) override
	{
		m_FPS = 1 / ts;
		if (Ancora::Input::IsKeyPressed(AE_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Ancora::Input::IsKeyPressed(AE_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (Ancora::Input::IsKeyPressed(AE_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		else if (Ancora::Input::IsKeyPressed(AE_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;

		if (Ancora::Input::IsKeyPressed(AE_KEY_LEFT))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		else if (Ancora::Input::IsKeyPressed(AE_KEY_RIGHT))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		Ancora::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Ancora::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Ancora::Renderer::BeginScene(m_Camera);

		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Ancora::OpenGLShader>(m_Shader)->Bind();
		std::dynamic_pointer_cast<Ancora::OpenGLShader>(m_Shader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.17f, y * 0.17f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Ancora::Renderer::Submit(m_Shader, m_VertexArray, transform);
			}
		}

		m_Texture->Bind(0);
		Ancora::Renderer::Submit(m_TextureShader, m_VertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(0.9f)));

		m_CloudTexture->Bind(0);
		Ancora::Renderer::Submit(m_TextureShader, m_VertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(0.9f)));

		// Triangle
		// Ancora::Renderer::Submit(m_BlackShader, m_TriangleVA);

		Ancora::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("FPS: %d", m_FPS);
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Ancora::Event& event) override
	{
	}
private:
	Ancora::Ref<Ancora::Shader> m_Shader;
	Ancora::Ref<Ancora::VertexArray> m_VertexArray;

	Ancora::Ref<Ancora::Shader> m_BlackShader, m_TextureShader;
	Ancora::Ref<Ancora::VertexArray> m_TriangleVA;

	Ancora::Ref<Ancora::Texture2D> m_Texture;
	Ancora::Ref<Ancora::Texture2D> m_CloudTexture;

	Ancora::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 2.0f;
	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 90.0f;
	int m_FPS;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.7f };
};


class Sandbox : public Ancora::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}

};

Ancora::Application* Ancora::CreateApplication()
{
	return new Sandbox();
}
