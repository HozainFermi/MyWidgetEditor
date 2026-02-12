#include "Scene.hpp"

namespace Styles {
		
	Scene::Scene(std::vector<MeshData>& meshes, std::unique_ptr<Helpers::Camera> camera, std::unique_ptr<Helpers::TextureContainer> textures)
		: _meshes(meshes),
		_camera(std::move(camera)),    
		_textures(std::move(textures)) 
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
		
		for (size_t i = 0; i < _meshes.size(); i++)
		{
			_textures.get()->ActivateAndBind();

			auto projection = glm::perspective(glm::radians(_camera.get()->Zoom), (float)800 / (float)600, 0.1f, 100.0f);
			auto view = _camera.get()->GetViewMatrix();
			
			_meshes[i].shader->Use();
			_meshes[i].shader->setFloat("u_time", (float)glfwGetTime());			
			_meshes[i].shader->setMat4("model", _meshes[i].model);
			_meshes[i].shader->setMat4("view", view);
			_meshes[i].shader->setMat4("projection", projection);
			
			_meshes[i].loader.get()->Draw();						
		}		
	}

	void Scene::AddMesh(MeshData&& mesh)
	{
		_meshes.push_back(std::move(mesh));
	}

	void Scene::RemoveMesh(int index)
	{
		_meshes.erase(_meshes.begin() + index);
	}

	void Scene::ProcessInputData()
	{

	}
}