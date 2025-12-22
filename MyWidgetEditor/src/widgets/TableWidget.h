#pragma once
#include "Widget.h"
#include <string>

namespace wg {

	enum class ContentsType { CT_Text, CT_FillButton };
	enum class DataType {CSV,JSON};
	enum class UpdateTrigger {
		TIMER,
		BUTTON_CLICK,
		KEY_PRESS,
		EXTERNAL_EVENT,
		MANUAL
	};
	class TableWidget : public Widget {
	private:
		ImGuiTableFlags flags;
		int max_cell_text_length_ = 256;
		int rows_ = 3;
		int cols_ = 3;
		std::string data_source_;
		DataType source_data_type_;
		UpdateTrigger trigger_;

	public:
		std::vector<std::vector<std::pair<ContentsType, std::string>>> table_content_;
	
		TableWidget(const std::string& name, const ImVec2& pos);

		bool UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id) override;
		void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
		void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;

		bool TryGetDataFromSource(){}
		
		void SetDataSource(std::string& source) { data_source_ = source; }
		void SetSourceDataType(DataType& type) { source_data_type_ = type; }
		void SetUpdateTrigger(UpdateTrigger& trigger_type) { trigger_ = trigger_type; }
		void SetColumn(int& value) { cols_= value; }
		void SetRows(int& value) { rows_ = value; }
		const int& GetRowsNumber() const { return rows_; }
		const int& GetColsNumber() const { return cols_; }
		const UpdateTrigger& GetUpdateTrigger() const { return trigger_; }
		const std::string& GetDataSource() const { return data_source_; }
		const DataType& GetSourceDataType() const { return source_data_type_; }

		nlohmann::json ToJson() const;
		void FromJson(const nlohmann::json& json);

	};

}

