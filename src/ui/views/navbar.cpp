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
		.name = "Editor",
		.dock = "Editor",
	},
	NavbarOption {
		.name = "Object Details",
		.dock = "Object Details",
	},
	NavbarOption {
		.name = "Navmesh",
		.dock = "Navmesh",
	},
	NavbarOption {
		.name = "Cameras",
		.dock = "Cameras",
	},
	NavbarOption {
		.name = "Lights",
		.dock = "Lights",
	},
	NavbarOption {
		.name = "Textures",
		.dock = "Textures",
	},
	NavbarOption {
		.name = "Transform",
		.dock = "Transform",
	},
	NavbarOption {
		.name = "Scenegraph",
		.dock = "Scenegraph",
	},
	NavbarOption {
		.name = "Debug",
		.dock = "Debug",
	},
	/*NavbarOption {
		.name = "Scene Info",
	},
	NavbarOption {
		.name = "Performance",
	},
	NavbarOption {
		.name = "World",
	},
	NavbarOption {
		.name = "Rendering",
	},
	NavbarOption {
		.name = "Models",
	},
	NavbarOption {
		.name = "Textures",
	},
	NavbarOption {
		.name = "Geo",
	},
	NavbarOption {
		.name = "Portal",
	},
	NavbarOption {
		.name = "Voxels",
	},
	NavbarOption {
		.name = "Debug",
	},*/
};

std::vector<NavbarOption> navbarOptionsGameplay = {
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
		.name = "HUD",
		.dock = "HUD",
	},
	NavbarOption {
		.name = "WATER",
		.dock = "WATER",
	},
};

std::vector<NavbarOption> navbarOptionsEditor = {
	NavbarOption {
		.name = "Hide",
		.dock = "",
	},
	NavbarOption {
		.name = "COLORS",
		.dock = "COLORS",
	},
	NavbarOption {
		.name = "Particle Viewer",
		.dock = "Particle Viewer",
	},
	
};

NavbarType strToNavbarType(std::string& layout){
  if (layout == "main"){
    return MAIN_EDITOR;
  }
  if (layout == "gameplay"){
    return GAMEPLAY_EDITOR;
  }
  if (layout == "editor"){
    return EDITOR_EDITOR;
  }
  modassert(false, std::string("invalid layout: ") + layout);
  return MAIN_EDITOR;
}


const float STYLE_UI_NAVBAR_FONTSIZE = 0.02f;
const float STYLE_UI_NAVBAR_ITEM_PADDING = 0.02f;
const float STYLE_UI_NAVBAR_PADDING = 0.02f;

Props createMenuOptions(NavbarType type, std::function<void(const char*)>& onClickNavbar){
	std::vector<NavbarOption>* navbarOptionsPtr = NULL;
	if (type == MAIN_EDITOR){
		navbarOptionsPtr = &navbarOptions;
	}else if (type == GAMEPLAY_EDITOR){
		navbarOptionsPtr = &navbarOptionsGameplay;
	}else if (type == EDITOR_EDITOR){
		navbarOptionsPtr = &navbarOptionsEditor;
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
