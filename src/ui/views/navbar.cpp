#include "./navbar.h"

struct NavbarOption {
	const char* name;
	const char* dock;
};

std::vector<NavbarOption> navbarOptions = {
	NavbarOption {
		.name = "Hide",
		.dock = "",
	},
	NavbarOption {
		.name = "MOVEMENT",
		.dock = "MOVEMENT",
	},
	NavbarOption {
		.name = "WEAPONS",
		.dock = "WEAPONS",
	},
	NavbarOption {
		.name = "TRIGGERS",
		.dock = "TRIGGERS",
	},
	NavbarOption {
		.name = "Particle Viewer",
		.dock = "Particle Viewer",
	},
	NavbarOption {
		.name = "Spawn",
		.dock = "Spawn",
	},
};

NavbarType strToNavbarType(std::string& layout){
  return MAIN_EDITOR;
}

const float STYLE_UI_NAVBAR_FONTSIZE = 0.02f;
const float STYLE_UI_NAVBAR_ITEM_PADDING = 0.02f;
const float STYLE_UI_NAVBAR_PADDING = 0.02f;

Props createMenuOptions(NavbarType type, std::function<void(const char*)>& onClickNavbar){
	std::vector<NavbarOption>* navbarOptionsPtr = NULL;
	if (type == MAIN_EDITOR){
		navbarOptionsPtr = &navbarOptions;
	}
	modassert(navbarOptionsPtr != NULL, "navbar type null");
  std::vector<ListComponentData> levels;
  for (auto &navbarOption : *navbarOptionsPtr){
  	levels.push_back(ListComponentData {
  		.name = navbarOption.name,
  		.onClick = [onClickNavbar, &navbarOption]() -> void {
  			onClickNavbar(navbarOption.dock);
  		}
  	});
  }

  float xoffset = -0.83f;
  float width = 2.f - xoffset - 0.2f;
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = xoffset },
      PropPair { .symbol = yoffsetSymbol,   .value = 1.f },
      PropPair { .symbol = minwidthSymbol,  .value = width },
      PropPair { .symbol = tintSymbol,      .value = styles.primaryColor },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = paddingSymbol,      .value = STYLE_UI_NAVBAR_PADDING },
      PropPair { .symbol = flowHorizontal, .value = UILayoutFlowPositive2 },
      PropPair { .symbol = flowVertical,     .value = UILayoutFlowNegative2 },
      PropPair { .symbol = fontsizeSymbol,     .value = STYLE_UI_NAVBAR_FONTSIZE },
      PropPair { .symbol = itemPaddingSymbol,     .value = STYLE_UI_NAVBAR_ITEM_PADDING },

    },
  };
  return levelProps;
}



Component navbarComponent {
  .draw = [](DrawingTools& drawTools, Props& props){	
  	auto onClick = fnStrFromProp(props, onclickSymbol);
  	modassert(onClick.has_value(), "navbar - need to provide on click value");
  	auto typePtr = typeFromProps<NavbarType>(props, valueSymbol);
  	auto navbarType = typePtr ? *typePtr : MAIN_EDITOR;
  	Props defaultProps { .props = {} };
  	return withPropsCopy(listComponent,  createMenuOptions(navbarType, onClick.value())).draw(drawTools, defaultProps);
  },
};
