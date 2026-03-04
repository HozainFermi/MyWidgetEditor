#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <Camera.h>
#include <GLFW/glfw3.h>
#include <Shader.hpp>
#include <Model.hpp>

namespace Styles {

	struct ModelData {
		std::unique_ptr<Helpers::Model> model;
		std::unique_ptr<Helpers::Shader> shader;
		glm::mat4 model_mat = glm::mat4(1.0f);
		std::filesystem::path path;
		std::string name;
	};

	class Scene {
	public:
		std::vector<ModelData> models_;
		std::unique_ptr<Helpers::Camera> camera_;

		glm::mat4 projection;
		glm::mat4 view;

		unsigned int SCR_WIDTH = 400;
		unsigned int SCR_HEIGHT = 300;

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
		void AddModel(const std::filesystem::path& path, const std::string& name);

		void ProcessInputData();
	};
}
