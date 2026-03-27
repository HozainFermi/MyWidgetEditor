#include "RenderWidget.h"



namespace rn {
	REGISTER_WIDGET(RenderWidget);


	RenderWidget::RenderWidget()
		:Widget("Unnamed", WidgetType::IMAGE, ImVec2(20, 20), ImVec2(400, 300)),
		scene_()
	{
		SetWidgetClass("RenderWidget");
	}

	RenderWidget::RenderWidget(const std::string& name, const ImVec2& pos)
		:Widget(name, WidgetType::IMAGE, pos, ImVec2(400, 300)),
		scene_()
	{
		SetWidgetClass("RenderWidget");
	}
	
	void RenderWidget::Render(ImDrawList* draw_list)
	{
		Widget::Render(draw_list);
	}

	void RenderWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
	{
		ImVec2 widget_size = GetSize();

		scene_.SCR_WIDTH = widget_size.x;
		scene_.SCR_HEIGHT = widget_size.y;

		// 1. Сохраняем текущий Viewport, чтобы не сбить ImGui
		GLint last_viewport[4];
		glGetIntegerv(GL_VIEWPORT, last_viewport);

		// 2. Рендерим сцену в FBO
		glBindFramebuffer(GL_FRAMEBUFFER, scene_.FBO);
		glViewport(0, 0, (GLsizei)widget_size.x, (GLsizei)widget_size.y);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (!scene_.models_.empty()) {
			scene_.models_[0].model_mat =
				glm::rotate(scene_.models_[0].model_mat,
					glm::radians(sin(static_cast<float>(glfwGetTime()))),
					glm::vec3(0.0f, 1.0f, 0.0f));
		}

		scene_.Draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. Восстанавливаем Viewport обратно для ImGui
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

		ImVec2 size = ImVec2(screen_max.x - screen_min.x - 10,
			screen_max.y - screen_min.y - 10);
		ImVec2 pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		if (ImGui::BeginChild("##render", widget_size)) {
			ImGui::Image((ImTextureID)(intptr_t)scene_.textureColorBuffer,
				ImVec2(widget_size.x, widget_size.y),
				ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
	}


	

	void RenderWidget::RenderSelectedMeshProps(int index) {
		return;
	}


	void RenderWidget::AddItem()
	{

	}

	void RenderWidget::RemoveItem()
	{

	}

	
	void RenderWidget::FromJson(const nlohmann::json& json)
	{
		Widget::FromJson(json);
		for (const auto& model : json["models"]) {
			scene_.AddModel(model["model_path"], model["vert_shader_path"], model["fragment_shader_path"], "");
		}

	}

	void RenderWidget::GeneratePreviewData()
	{

	}

}