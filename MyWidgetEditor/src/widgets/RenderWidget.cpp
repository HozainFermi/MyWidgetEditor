#include "RenderWidget.h"



namespace wg {
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

	bool RenderWidget::UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id)
	{
		return Widget::UpdateInteraction(canvas_p0,canvas_size,widget_id);
	}

	void RenderWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0)
	{
		Widget::Render(draw_list, canvas_p0);
	}

	void RenderWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
	{
		ImVec2 widget_size = GetSize();

		// 1. Сохраняем текущий Viewport, чтобы не сбить ImGui
		GLint last_viewport[4];
		glGetIntegerv(GL_VIEWPORT, last_viewport);

		// 2. Рендерим сцену в FBO
		glBindFramebuffer(GL_FRAMEBUFFER, scene_.FBO);
		glViewport(0, 0, (GLsizei)widget_size.x, (GLsizei)widget_size.y);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene_.meshes_[0].model = glm::rotate(scene_.meshes_[0].model, glm::radians(sin((float)glfwGetTime()) * 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));

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


	void RenderWidget::RenderProperties()
	{

	}

	void RenderWidget::AddItem()
	{

	}

	void RenderWidget::RemoveItem()
	{

	}

	nlohmann::json RenderWidget::ToJson() const
	{
		return nlohmann::json();
	}

	void RenderWidget::FromJson(const nlohmann::json& json)
	{

	}

	void RenderWidget::GeneratePreviewData()
	{

	}

}