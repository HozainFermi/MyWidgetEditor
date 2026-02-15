#include "Scene.hpp"
#include <shapes/cube.hpp>

namespace Styles {
	Scene::Scene()
	{		
		MeshData data;
		data.loader = std::make_unique<Helpers::MeshLoader>(Shapes::cube,5*sizeof(Shapes::cube[0]) );
		data.shader = std::make_unique<Helpers::Shader>("vertex.glsl", "fragment.glsl");
		data.model = glm::mat4(1.0f);

		meshes_.push_back(std::move(data));

		glGenBuffers(1, &FBO);
	}
	Scene::Scene(std::vector<MeshData>& meshes, std::unique_ptr<Helpers::Camera> camera, std::unique_ptr<Helpers::TextureContainer> textures)
		: meshes_(meshes),
		camera_(std::move(camera)),    
		textures_(std::move(textures)) 
	{
		glGenBuffers(1, &FBO);		
	}
	Scene::~Scene()
	{		
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