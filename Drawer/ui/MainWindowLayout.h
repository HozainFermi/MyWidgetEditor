#pragma once
#include <imgui.h>
#include "../managers/RuntimeWidgetManager.h"
#include <Scene.hpp>
#include <GLFW/glfw3.h>


namespace rn {

	 class MainWindowLayout {
     private:
         Styles::Scene background_scene_{};
         static RuntimeWindowProperties window_props_;
                         	     
     public:
         
         MainWindowLayout()
         {
             std::vector<Helpers::Vertex> vertices_{

             Helpers::Vertex{
             glm::vec3{-1.f,1.f,0.0f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{0.f,1.f} },
             Helpers::Vertex{
             glm::vec3{1.f,1.f,0.f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{1.0f,1.0f} },
             Helpers::Vertex{
             glm::vec3{-1.f,-1.f,0.0f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{0.f,0.f} },
             
              Helpers::Vertex{
             glm::vec3{1.f,1.f,0.f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{1.0f,1.0f} },
              Helpers::Vertex{
             glm::vec3{-1.f,-1.f,0.0f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{0.f,0.f} },
              Helpers::Vertex{
             glm::vec3{1.0f,-1.f,0.0f},
             glm::vec3{1.f,1.f,1.0f},
             glm::vec2{1.0f,0.0f} },

             };
             std::vector<unsigned int> EBOpoints_ = {
                 0,1,2,
                 1,2,5
             };

             Helpers::Mesh background_mesh_{ vertices_,EBOpoints_ };

             std::unique_ptr model = std::make_unique<Helpers::Model>();
             model->AddMesh(background_mesh_);

             std::unique_ptr shaders = std::make_unique<Helpers::Shader>(window_props_.vertex_GLSLshader_file, window_props_.frag_GLSLshader_file);
             Styles::ModelData model_data{ std::move(model), std::move(shaders) };

             //background_scene_.FBO = 1;
             //background_scene_.textureColorBuffer = 1;
             //background_scene_.rbo = 1;

             background_scene_.models_.push_back(std::move(model_data));
         }

        static MainWindowLayout* Get(RuntimeWindowProperties props) {
            window_props_ = props;
            static MainWindowLayout instance;
            return &instance;
        }
        MainWindowLayout(const MainWindowLayout&) {
            throw std::logic_error("Can not copy Editor - it`s singleton");
        }
        void operator=(const MainWindowLayout&) {
            throw std::logic_error("Can not copy Editor - it`s singleton");
        }
        MainWindowLayout(MainWindowLayout&&) = delete;
        void operator=(const MainWindowLayout&&) = delete;
                                                                                       
		void Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window, RuntimeWidgetManager* manager);
        
	};
}