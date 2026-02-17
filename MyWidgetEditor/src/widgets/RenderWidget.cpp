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
		scene_.meshes_[0].model = glm::rotate(scene_.meshes_[0].model, glm::radians(sin((float)glfwGetTime()) ), glm::vec3(0.0f, 1.0f, 0.0f));

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
		static float Xoffset;
		static float Yoffset;
		const char* preview_value = scene_.meshes_[selected_idx].meshType.c_str();
		std::string meshID;

		ImGui::Text("==Render Settings==");
		ImGui::Separator();

		if (ImGui::BeginCombo("Select Mesh", preview_value)) {
			for (int i = 0; i < scene_.meshes_.size(); ++i) {
				const bool is_selected = (selected_idx == i);
				meshID = scene_.meshes_[i].meshType + "##" + std::to_string(i);

				// Рисуем элемент и проверяем, не кликнули ли по нему
				if (ImGui::Selectable(meshID.c_str(), is_selected)) {
					selected_idx = i;
					//scene_.meshes_[i].isSelected = true;
				}

				// Устанавливаем фокус на выбранный элемент 
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();

		}
		
		if (ImGui::Button("Add Mesh")) {
			ImGui::OpenPopup("mesh_selector"); 
		}
		
		if (ImGui::BeginPopup("mesh_selector")) {
			static const char* items[] = { "Cube", "Plane", "Teapot" };
			static const float* arrays[] = {Shapes::Cube::cube, Shapes::Plane::plane};

			for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
				if (ImGui::Selectable(items[i])) {
					
					Styles::MeshData data;
					data.loader = std::make_unique<Helpers::MeshLoader>(Shapes::Cube::cube, 5);
					data.loader.get()->AddAttribPointer(3, 5 * sizeof(float), nullptr);
					data.loader.get()->AddAttribPointer(2, 5 * sizeof(float), (void*)(3 * sizeof(float)));
					data.meshType = Shapes::Cube::type;

					data.shader = std::make_unique<Helpers::Shader>(ShaderSources::Test_vertex, ShaderSources::Test_fragment);
					data.model = glm::translate(scene_.meshes_[scene_.meshes_.size()-1].model, glm::vec3(0.0f, 0.0f, -1.0f));
					scene_.meshes_.push_back(std::move(data));

					printf("Selected: %s\n", items[i]);
				}
			}
			ImGui::EndPopup();
		}

		RenderSelectedMeshProps(selected_idx);

			ImGui::Separator();
			ImGui::Text("Camera");
			ImGui::Separator();

			ImGui::DragFloat("Zoom:", &scene_.camera_.get()->Zoom, scene_.camera_->MouseSensitivity);
			if (ImGui::DragFloat("X offset:", &Xoffset, scene_.camera_->MouseSensitivity)) {
				scene_.camera_->ProcessMouseMovement(Xoffset,Yoffset);
			}
			if (ImGui::DragFloat("Y offset:", &Yoffset, scene_.camera_->MouseSensitivity)) {
				scene_.camera_->ProcessMouseMovement(Xoffset, Yoffset);
			}
			
			ImGui::Text("Camera Position:");
			ImGui::DragFloat("X:", &scene_.camera_.get()->Position.x, 0.01f);
			ImGui::DragFloat("Y:", &scene_.camera_.get()->Position.y, 0.01f);


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