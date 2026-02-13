#include "RenderWidget.h"



namespace wg {
	REGISTER_WIDGET(RenderWidget);


	RenderWidget::RenderWidget()
		:Widget("Unnamed", WidgetType::IMAGE, ImVec2(20, 20), ImVec2(400, 300))
	{

		SetWidgetClass("RenderWidget");
	}

	RenderWidget::RenderWidget(const std::string& name, const ImVec2& pos)
		:Widget("Unnamed", WidgetType::IMAGE, pos, ImVec2(400, 300))
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
		glBindFramebuffer(GL_FRAMEBUFFER, scene.FBO);
		glViewport(0, 0, widget_size.x, widget_size.y);
		scene.Draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		if (ImGui::BeginTable(("##render_" + GetId()).c_str(),
			(int)columns_.size(),
			flags_,
			content_size)) {

		ImGui::Image((void*)(intptr_t)FBO, widget_size, ImVec2(0, 1), ImVec2(1, 0));
		}


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