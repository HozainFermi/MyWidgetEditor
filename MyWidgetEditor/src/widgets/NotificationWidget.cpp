#include "NotificationWidget.h"

namespace wg {	
	REGISTER_WIDGET(NotificationWidget);

	NotificationWidget::NotificationWidget()
	 :Widget("Notification", WidgetType::CUSTOM, ImVec2(20, 20), ImVec2(400, 300))		
	{
		SetWidgetClass("NotificationWidget");
	}

	NotificationWidget::NotificationWidget(const std::string& name, const ImVec2& pos)
	 :Widget(name, WidgetType::CUSTOM, pos, ImVec2(400, 300))
	{
		SetWidgetClass("NotificationWidget");
	}

	void NotificationWidget::RenderProperties()
	{
	}
	
	void NotificationWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0)
	{
		Widget::Render(draw_list,canvas_p0);
	}

	void NotificationWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
	{

	}

	std::vector<PortDesc> NotificationWidget::GetInputPorts() const
	{
		return {
			PortDesc{ .name="data",		.label="Data",    .type="json",  .is_input=true },
			PortDesc{ .name="refresh",  .label="Refresh", .type="event", .is_input=true }
		};
	}

	std::vector<PortDesc> NotificationWidget::GetOutputPorts() const
	{
		return std::vector<PortDesc>();
	}

	void NotificationWidget::OnInput(const std::string& port, const WidgetValue& value)
	{
	}
	
	nlohmann::json NotificationWidget::ToJson() const
	{
		return nlohmann::json();
	}

	void NotificationWidget::FromJson(const nlohmann::json& json)
	{
	}

	


}