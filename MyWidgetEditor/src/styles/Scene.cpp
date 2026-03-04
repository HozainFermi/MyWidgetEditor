#include "Scene.hpp"
#include <shaders/shaderSources.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Styles {

	Scene::Scene()
	{
		camera_ = std::make_unique<Helpers::Camera>(
			glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			Helpers::YAW,
			Helpers::PITCH);

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &textureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Scene::AddModel(const std::filesystem::path& path, const std::string& name)
	{
		ModelData data;
		data.path = path;
		data.name = name.empty() ? path.filename().string() : name;
		data.model = std::make_unique<Helpers::Model>(path);
		data.shader = std::make_unique<Helpers::Shader>(ShaderSources::Test_vertex, ShaderSources::Test_fragment);
		data.model_mat = glm::mat4(1.0f);
		models_.push_back(std::move(data));
	}

	void Scene::Draw()
	{
		if (!camera_)
			return;

		time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;

		if (models_.empty())
			return;

		projection = glm::perspective(glm::radians(camera_->Zoom),
			static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
			0.1f, 100.0f);
		view = camera_->GetViewMatrix();

		for (auto& m : models_)
		{
			if (!m.shader || !m.model)
				continue;

			m.shader->Use();
			m.shader->setMat4("model", m.model_mat);
			m.shader->setMat4("view", view);
			m.shader->setMat4("projection", projection);
			m.model->Draw(*m.shader);
		}
	}

	void Scene::ProcessInputData()
	{
	}
}