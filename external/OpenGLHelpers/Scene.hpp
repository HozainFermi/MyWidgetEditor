#pragma once
#include <glad/gl.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <Camera.h>
#include <Shader.hpp>
#include <Model.hpp>
#include <json.hpp>
#include <GLFW/glfw3.h>

namespace Styles {

	struct ModelData {
		std::unique_ptr<Helpers::Model> model;
		std::unique_ptr<Helpers::Shader> shader;
		glm::mat4 model_mat = glm::mat4(1.0f);
		std::filesystem::path path;
		std::string name;

		nlohmann::json ToJson() const {
			nlohmann::json json;
			json["model_path"] = path.string();
			json["vert_shader_path"] = shader->vertex_shader_path;
			json["fragment_shader_path"] = shader->fragment_shader_path;
			return json;
		}
		
	};

	class Scene {
	public:
		std::vector<ModelData> models_;
		std::unique_ptr<Helpers::Camera> camera_;

		glm::mat4 projection;
		glm::mat4 view;

		unsigned int SCR_WIDTH = 400;
		unsigned int SCR_HEIGHT = 400;

	public:
		float time = 0.0f;
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;
		unsigned int FBO = 0;
		unsigned int textureColorBuffer = 0;
		unsigned int rbo = 0;

		Scene();
		~Scene() = default;

		void Draw();
		void ResetState();
		void AddModel(const std::filesystem::path& model_path,
			const std::filesystem::path& vert_shader_path,
			const std::filesystem::path& frag_shader_path,
			const std::string& name);

		void ProcessInputData();
	};
}
