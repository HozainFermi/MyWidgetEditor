#include "RenderWidget.h"
#include <IconText.h>



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

		ImGui::PushID(this);
		if (ImGui::BeginChild("##render", widget_size)) {
			ImGui::Image((ImTextureID)(intptr_t)scene_.textureColorBuffer,
				ImVec2(widget_size.x, widget_size.y),
				ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
		ImGui::PopID();
		
	}


	void RenderWidget::RenderProperties()
	{
		static int selected_idx = 0;
		static float Xoffset = 0.0f;
		static float Yoffset = 0.0f;
		
		static std::filesystem::path shadersFolderPath = std::string(ASSETS_SOURCE_DIR) + "/shaders";
		static std::filesystem::path modelsFolderPath = std::string(ASSETS_SOURCE_DIR) + "/models";

		static std::string modelPath= modelsFolderPath.string()  + "/backpack/backpack.obj";		 modelPath.resize(256);
		static std::string VERTbuf =  shadersFolderPath.string() + "/backpack/1.model_loading.vert"; VERTbuf.resize(256);
		static std::string FRAGbuf =  shadersFolderPath.string() + "/backpack/1.model_loading.frag"; FRAGbuf.resize(256);
		
		static std::vector<std::filesystem::path> frag_shaders_paths{};		
		static std::vector<std::filesystem::path> vert_shaders_paths{};		
		static std::vector<std::filesystem::path> models_paths{};

		ImGui::Text("==Render Settings==");
		ImGui::Separator();

		ExplorerButton(modelPath,models_paths,modelsFolderPath,PathType::Model);
		ImGui::SameLine();
		ImGui::InputText("Model path##model", modelPath.data(), modelPath.capacity());
		
		
		ExplorerButton(VERTbuf, vert_shaders_paths, shadersFolderPath, PathType::VertexShader);
		ImGui::SameLine();
		if (ImGui::InputText("Vert Shader##model", VERTbuf.data(), VERTbuf.capacity())) {}
		
		
		ExplorerButton(FRAGbuf, frag_shaders_paths, shadersFolderPath, PathType::FragmentShader);
		ImGui::SameLine();
		if (ImGui::InputText("Frag Shader##model", FRAGbuf.data(), FRAGbuf.capacity())) {}
		

		if (ImGui::Button("Load model")) {
			scene_.AddModel(modelPath,VERTbuf,FRAGbuf,"");
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
			ImGui::DragFloat("Z:", &scene_.camera_.get()->Position.z, 0.01f);
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
		auto json = Widget::ToJson();		
		nlohmann::json models_json = nlohmann::json::array();
		nlohmann::json camera_json;
		for (const auto& model : scene_.models_) {
			models_json.push_back(model.ToJson());			
		}
		json["models"] = models_json;
		
		camera_json["x"] = scene_.camera_->Position.x;
		camera_json["y"] = scene_.camera_->Position.y;
		camera_json["z"] = scene_.camera_->Position.z;
		camera_json["zoom"] = scene_.camera_->Zoom;
		
		json["camera"] = camera_json;
		return json;
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

	void RenderWidget::ExplorerButton(std::string& currentPath, std::vector<std::filesystem::path>& paths, std::filesystem::path& folderPath, PathType type) {
		
		ImGui::PushID(PathTypeToString(type).c_str());
		if (ImGui::Button(ICON(ICON_FOLDER), ImVec2(35, 20))) {
			paths.clear();
			for (const auto& folder : std::filesystem::directory_iterator(folderPath))
			{
				if (folder.is_directory()) {					
						for (const auto& file : std::filesystem::directory_iterator(folder)) {							
							if (type == PathType::Model) {
								if (file.path().extension() == ".obj" || file.path().extension() == ".gltf") {
									paths.push_back(file.path());
								}
							}
							if (type == PathType::VertexShader) {
								if (file.path().extension() == ".vert") {
									paths.push_back(file.path());
								}
							}
							if (type == PathType::FragmentShader) {
								if (file.path().extension() == ".frag") {
									paths.push_back(file.path());
								}
							}
						}										
				}
			}
			ImGui::OpenPopup("select_explorer_popup");
		}

		if (ImGui::BeginPopup("select_explorer_popup")) {
			for (size_t i = 0; i < paths.size(); ++i) {				
				if (ImGui::Selectable(paths[i].filename().string().c_str())) { currentPath = paths[i].generic_string(); }
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

}