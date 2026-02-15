#pragma once
#include <vector>
#include <memory>
#include <MeshLoader.hpp>
#include <Camera.h>
#include <TextureContainer.hpp>
#include <GLFW/glfw3.h>
#include <Shader.hpp>


namespace Styles {

	
	struct MeshData {
		std::unique_ptr<Helpers::MeshLoader> loader;
		std::unique_ptr<Helpers::Shader> shader;

		glm::mat4 model = glm::mat4(1.0f);		
	};

	class Scene {
	private:
		std::vector<MeshData> meshes_;
		std::unique_ptr<Helpers::Camera> camera_;
		std::unique_ptr<Helpers::TextureContainer> textures_;		
		
	public:
		float time;
		float deltaTime;
		float lastFrame;
		unsigned int FBO=0;

		Scene();
		Scene(std::vector<MeshData>& meshes, std::unique_ptr<Helpers::Camera> camera,std::unique_ptr<Helpers::TextureContainer> textures);
		~Scene();
		
		void Draw();		
		void AddMesh(MeshData&& mesh);
		void RemoveMesh(int index);

		void ProcessInputData();

		

	};
}
//Textures.ActivateAndBind();
//
//time = glfwGetTime();
//deltaTime = time - lastFrame;
//lastFrame = time;
//
//processInput(window);
//
////cube
//LightShader.Use();
//projection = glm::mat4(1.0f);
//view = glm::mat4(1.0f);
//model = glm::mat4(1.0f);
//
//projection = glm::perspective(glm::radians(Cam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//view = Cam.GetViewMatrix();
//model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
//
//LightShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
//LightShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
//LightShader.setFloat("u_time", (float)glfwGetTime());
//LightShader.setMat4("model", model);
//LightShader.setMat4("view", view);
//LightShader.setMat4("projection", projection);
//
//FirstContainer.Draw();