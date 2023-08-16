#ifndef MOD_AFTERWORLD_COMPONENTS_RADIOBUTTON
#define MOD_AFTERWORLD_COMPONENTS_RADIOBUTTON

#include "./common.h"
#include "../../global.h"

struct RadioButton {
  bool selected;
  bool hovered;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

struct RadioButtonContainer {
	int selectedRadioButtonIndex;
	std::vector<RadioButton> radioButtons;
};

extern Component radioButtons;

#endif