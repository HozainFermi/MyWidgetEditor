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


	void RenderWidget::RenderProperties()
	{
		static int selected_idx = 0;
		static float Xoffset = 0.0f;
		static float Yoffset = 0.0f;
		static char modelPath[260] = "C:/Users/dedde/source/repos/MyWidgetEditor/assets/models/backpack/backpack.obj";

		ImGui::Text("==Render Settings==");
		ImGui::Separator();

		ImGui::InputText("Model path", modelPath, IM_ARRAYSIZE(modelPath));
		if (ImGui::Button("Load model")) {
			scene_.AddModel(modelPath, "");
			if (!scene_.models_.empty()) {
				selected_idx = static_cast<int>(scene_.models_.size()) - 1;
			}
		}

		if (!scene_.models_.empty()) {
			if (selected_idx >= static_cast<int>(scene_.models_.size())) {
				selected_idx = static_cast<int>(scene_.models_.size()) - 1;
			}

			const char* preview_value = scene_.models_[selected_idx].name.c_str();

			if (ImGui::BeginCombo("Select Model", preview_value)) {
				for (int i = 0; i < static_cast<int>(scene_.models_.size()); ++i) {
					const bool is_selected = (selected_idx == i);
					std::string label = scene_.models_[i].name + "##" + std::to_string(i);

					if (ImGui::Selectable(label.c_str(), is_selected)) {
						selected_idx = i;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		} else {
			ImGui::Text("No models loaded");
		}

		ImGui::Separator();
		ImGui::Text("Camera");
		ImGui::Separator();

		if (scene_.camera_) {
			ImGui::DragFloat("Zoom:", &scene_.camera_.get()->Zoom, scene_.camera_->MouseSensitivity);
			if (ImGui::DragFloat("X offset:", &Xoffset, scene_.camera_->MouseSensitivity)) {
				scene_.camera_->ProcessMouseMovement(Xoffset, Yoffset);
			}
			if (ImGui::DragFloat("Y offset:", &Yoffset, scene_.camera_->MouseSensitivity)) {
				scene_.camera_->ProcessMouseMovement(Xoffset, Yoffset);
			}

			ImGui::Text("Camera Position:");
			ImGui::DragFloat("X:", &scene_.camera_.get()->Position.x, 0.01f);
			ImGui::DragFloat("Y:", &scene_.camera_.get()->Position.y, 0.01f);
		}
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