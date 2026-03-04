#pragma once
#include <vector>
#include <memory>
#include <MeshLoader.hpp>
#include <Camera.h>
#include <TextureContainer.hpp>
#include <GLFW/glfw3.h>
#include <Shader.hpp>
#include <Model.hpp>


namespace Styles {

	
	struct ModelData {
		std::unique_ptr<Helpers::Model> model;
		std::unique_ptr<Helpers::Shader> shader;
		glm::mat4 model_mat = glm::mat4(1.0f);
				
		//bool isSelected=false;
	};

	class Scene {
	public:
		std::vector<std::unique_ptr<Helpers::Model>> models_;
		std::unique_ptr<Helpers::Camera> camera_;
		

		glm::mat4 projection;
		glm::mat4 view;

		unsigned int SCR_WIDTH=400;
		unsigned int SCR_HEIGHT=300;
		
	public:
		float time;
		float deltaTime;
		float lastFrame;
		unsigned int FBO=0;
		unsigned int textureColorBuffer = 0;
		unsigned int rbo=0;

		Scene();
		Scene(std::vector<MeshData>&& meshes, std::unique_ptr<Helpers::Camera> camera,std::unique_ptr<Helpers::TextureContainer> textures);
		~Scene()=default;
		
		void Draw();		
		void AddMesh(MeshData&& mesh);
		void RemoveMesh(int index);

		void ProcessInputData();

		

	};
}
