#pragma once
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "../managers/RuntimeWidgetManager.h"


namespace rn {

	 class MainWindowLayout {

     private:
         MainWindowLayout() = default;
	     
     public:		
        static MainWindowLayout* Get() {
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