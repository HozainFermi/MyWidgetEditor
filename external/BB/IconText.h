#pragma once
#define ICON_MIN 0xe000 
#define ICON_MAX 0xe0fe

#define ICON_ACTIVITY u8"\ue000"

#define ICON_ALIGN_CENTER u8"\ue005"
#define ICON_ALIGN_LEFT u8"\ue007"
#define ICON_ALIGN_RIGHT u8"\ue008"
#define ICON_UNDERLINE u8"\ue0e3"
#define ICON_ITALIC u8"\ue078"

#define ICON_EDIT u8"\ue058"
#define ICON_FILE_PLUS u8"\ue05f"
#define ICON_FILE u8"\ue061"
#define ICON_SAVE u8"\ue0b7"


#define ICON(str) reinterpret_cast<const char*>(str)
