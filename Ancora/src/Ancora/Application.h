#pragma once

#include "Core.h"

#include "Window.h"
#include "Ancora/LayerStack.h"
#include "Events/Event.h"
#include "Ancora/Events/ApplicationEvent.h"
#include "Ancora/ImGui/ImGuiLayer.h"

namespace  Ancora {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();

}
