#pragma once
#include "Widget.h"
#include <string>

namespace wg {

	enum ContentsType { CT_Text, CT_FillButton };
	enum DataType {CSV,JSON};

	class TableWidget : public Widget {
	private:
		ImGuiTableFlags flags;
		int max_cell_text_length_ = 256;
		int rows_ = 3;
		int cols_ = 3;
		
		std::vector<std::vector<std::pair<ContentsType, std::string>>> table_content_;

		std::string buffer_;
		std::string label_;
		std::string hint_;

	};

}
