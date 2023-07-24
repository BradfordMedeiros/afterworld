#ifndef MOD_AFTERWORLD_COMPONENTS_RADIOBUTTON
#define MOD_AFTERWORLD_COMPONENTS_RADIOBUTTON

#include "./common.h"

struct RadioButton {
  bool selected;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

std::vector<RadioButton> createRadioButtons();
BoundingBox2D drawRadioButtons(std::vector<RadioButton> radioButtons, float xoffset, float yoffset, float width, float height);

struct RadioButtonContainer {
	int selectedRadioButtonIndex;
	std::vector<RadioButton> radioButtons;
};

extern RadioButtonContainer radioButtonContainer;

extern Component radioButtonSelector;

#endif