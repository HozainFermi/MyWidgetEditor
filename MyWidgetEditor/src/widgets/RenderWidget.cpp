#include "RenderWidget.h"



namespace wg {
	REGISTER_WIDGET(RenderWidget);


	RenderWidget::RenderWidget()
		:Widget("Unnamed", WidgetType::IMAGE, ImVec2(20, 20), ImVec2(400, 300)),
		scene_()
	{		
		SetWidgetClass("RenderWidget");
	}

	RenderWidget::RenderWidget(const std::string& name, const ImVec2& pos)
		:Widget(name, WidgetType::IMAGE, pos, ImVec2(400, 300)),
		 scene_()
	{		
		SetWidgetClass("RenderWidget");
	}

	bool RenderWidget::UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id)
	{
		return Widget::UpdateInteraction(canvas_p0,canvas_size,widget_id);
	}

	void RenderWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0)
	{
		Widget::Render(draw_list, canvas_p0);
	}

	void RenderWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
	{
		ImVec2 widget_size = GetSize();
		glBindFramebuffer(GL_FRAMEBUFFER, scene_.FBO);
		glViewport(0, 0, widget_size.x, widget_size.y);
		scene_.Draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		std::cout << "TEST";
		if (ImGui::BeginChild("##render", widget_size)) {
			ImGui::Image((void*)(intptr_t)scene_.textureColorBuffer,
				ImVec2(widget_size.x, widget_size.y),
				ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();


	}

	void RenderWidget::RenderProperties()
	{

	}

	void RenderWidget::AddItem()
	{

	}

	void RenderWidget::RemoveItem()
	{

	}

	nlohmann::json RenderWidget::ToJson() const
	{
		return nlohmann::json();
	}

	void RenderWidget::FromJson(const nlohmann::json& json)
	{

	}

	void RenderWidget::GeneratePreviewData()
	{

	}

}