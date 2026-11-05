#include "RegisterAllWidgets.h"
#include "core/WidgetFactory.h"
#include "nodes/FilterNode.h"
#include "nodes/MapNode.h"
#include "nodes/ShaderParamsNode.h"
#include "ActivityTrackerWidget.h"
#include "InputTextWidget.h"
#include "NotificationWidget.h"
#include "PlotWidget.h"
#include "RenderWidget.h"
#include "TableWidget.h"
#include "TextWidget.h"
#include "WeatherWidget.h"

namespace core {

    void RegisterAllWidgets() {
        WidgetFactory::Register<ActivityTrackerWidget>("ActivityTrackerWidget");
        WidgetFactory::Register<InputTextWidget>("InputTextWidget");
        WidgetFactory::Register<NotificationWidget>("NotificationWidget");
        WidgetFactory::Register<PlotWidget>("PlotWidget");
        WidgetFactory::Register<RenderWidget>("RenderWidget");
        WidgetFactory::Register<TableWidget>("TableWidget");
        WidgetFactory::Register<TextWidget>("TextWidget");
        WidgetFactory::Register<WeatherWidget>("WeatherWidget");

        // Узлы логики - только на вкладке Logic
        WidgetFactory::Register<FilterNode>("FilterNode", WidgetCategory::Logic);
        WidgetFactory::Register<MapNode>("MapNode", WidgetCategory::Logic);
        WidgetFactory::Register<ShaderParamsNode>("ShaderParamsNode", WidgetCategory::Logic);
    }
}
