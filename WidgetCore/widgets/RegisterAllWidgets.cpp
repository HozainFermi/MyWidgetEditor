#include "RegisterAllWidgets.h"
#include "core/WidgetFactory.h"
#include "InputTextWidget.h"
#include "PlotWidget.h"
#include "RenderWidget.h"
#include "TableWidget.h"
#include "TextWidget.h"

namespace core {

    void RegisterAllWidgets() {
        WidgetFactory::Register<InputTextWidget>("InputTextWidget");
        WidgetFactory::Register<PlotWidget>("PlotWidget");
        WidgetFactory::Register<RenderWidget>("RenderWidget");
        WidgetFactory::Register<TableWidget>("TableWidget");
        WidgetFactory::Register<TextWidget>("TextWidget");
    }
}
