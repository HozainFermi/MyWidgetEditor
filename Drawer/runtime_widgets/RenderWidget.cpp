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
		if (visibility_ == false) return;
		ImVec2 widget_size = GetSize();
		scene_.SaveCurrentState();

		scene_.SCR_WIDTH = widget_size.x;
		scene_.SCR_HEIGHT = widget_size.y;

		scene_.ResizeFramebuffer(static_cast<unsigned int>(widget_size.x),
								static_cast<unsigned int>(widget_size.y));

		// Рендерим сцену в FBO
		glBindFramebuffer(GL_FRAMEBUFFER, scene_.FBO);
		glViewport(0, 0, (GLsizei)widget_size.x, (GLsizei)widget_size.y);
		
		if (window_props->frag_GLSLshader_file.empty()) {
			glClearColor(window_props->bg_color_float[0], window_props->bg_color_float[1], window_props->bg_color_float[2], window_props->bg_color_float[3]);
		}
		else {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (!scene_.models_.empty()) {
			scene_.models_[0].model_mat =
				glm::rotate(scene_.models_[0].model_mat,
					glm::radians(sin(static_cast<float>(glfwGetTime()))),
					glm::vec3(0.0f, 1.0f, 0.0f));
		}

		scene_.Draw();
		scene_.RestorePrevState();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		ImVec2 size = ImVec2(screen_max.x - screen_min.x - 10,
			screen_max.y - screen_min.y - 10);
		ImVec2 pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::PushID(this);
		if (ImGui::BeginChild("##render", widget_size)) {
			
			ImVec2 avail = ImGui::GetContentRegionAvail();
			ImGui::Image((ImTextureID)(intptr_t)scene_.textureColorBuffer,
				avail,
				ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
		ImGui::PopID();

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
		auto camera_json = json["camera"];
		scene_.camera_->Position.x = camera_json.value("x", 0.0f);
		scene_.camera_->Position.y = camera_json.value("y", 0.0f);
		scene_.camera_->Position.z = camera_json.value("z", 0.0f);
		scene_.camera_->Zoom = camera_json.value("zoom", 45.0f);

		for (const auto& model : json["models"]) {
			scene_.AddModel(
				model.value("model_path", ""),
				model.value("vert_shader_path", ""),
				model.value("fragment_shader_path", ""),
				"");
		}
	}

	void RenderWidget::GeneratePreviewData()
	{

	}

}