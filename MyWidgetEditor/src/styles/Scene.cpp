#include "Scene.hpp"
#include <shapes/cube.hpp>
#include <shaders/shaderSources.hpp>

namespace Styles {
	Scene::Scene()
	{		
		MeshData data;
		camera_ = std::make_unique<Helpers::Camera>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), Helpers::YAW, Helpers::PITCH);
		textures_ = std::make_unique<Helpers::TextureContainer>();

		data.loader = std::make_unique<Helpers::MeshLoader>(Shapes::cube,5);
		data.loader.get()->AddAttribPointer(3, 5 * sizeof(float), nullptr);
		data.loader.get()->AddAttribPointer(2, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		data.shader = std::make_unique<Helpers::Shader>(ShaderSources::Test_vertex, ShaderSources::Test_fragment);
		data.model = glm::mat4(1.0f);

		meshes_.push_back(std::move(data));

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// 1. Создаем текстуру, куда будет идти рендер
		glGenTextures(1, &textureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 400, 300, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Прикрепляем текстуру к FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
			
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 400, 300);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	Scene::Scene(std::vector<MeshData>&& meshes, std::unique_ptr<Helpers::Camera> camera, std::unique_ptr<Helpers::TextureContainer> textures) 
		:meshes_(std::move(meshes)),
		camera_(std::move(camera)),    
		textures_(std::move(textures)) 
	{

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// 1. Создаем текстуру, куда будет идти рендер
		glGenTextures(1, &textureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 400, 300, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Прикрепляем текстуру к FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 400, 300);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	void Scene::Draw()
	{
		time = glfwGetTime();
		deltaTime = time - lastFrame;
		lastFrame = time;
		
		for (size_t i = 0; i < meshes_.size(); i++)
		{
			textures_.get()->ActivateAndBind();
			
			auto projection = glm::perspective(glm::radians(camera_.get()->Zoom), (float)800 / (float)600, 0.1f, 100.0f);
			auto view = camera_.get()->GetViewMatrix();			

			meshes_[i].shader->Use();
			meshes_[i].shader->setFloat("u_time", (float)glfwGetTime());			
			meshes_[i].shader->setMat4("model", meshes_[i].model);
			meshes_[i].shader->setMat4("view", view);
			meshes_[i].shader->setMat4("projection", projection);
			
			meshes_[i].loader.get()->Draw();						
		}		
	}

	void Scene::AddMesh(MeshData&& mesh)
	{
		meshes_.push_back(std::move(mesh));
	}

	void Scene::RemoveMesh(int index)
	{
		meshes_.erase(meshes_.begin() + index);
	}

	void Scene::ProcessInputData()
	{

	}
}