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
		.name = "Heightmap",
	},
	NavbarOption {
		.name = "Voxels",
	},
	NavbarOption {
		.name = "Debug",
	},*/
};




Props createMenuOptions(std::function<void(const char*)>& onClickNavbar){
  std::vector<ListComponentData> levels;
  for (auto &navbarOption : navbarOptions){
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
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 2.f) },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      //PropPair { .symbol = paddingSymbol,      .value = 0.02f },
      PropPair { .symbol = flowHorizontal, .value = UILayoutFlowPositive2 },
      PropPair { .symbol = flowVertical,     .value = UILayoutFlowNegative2 },
    },
  };
  return levelProps;
}



Component navbarComponent {
  .draw = [](DrawingTools& drawTools, Props& props){	
  	auto onClick = fnStrFromProp(props, onclickSymbol);
  	modassert(onClick.has_value(), "navbar - need to provide on click value");
  	Props defaultProps { .props = {} };
  	return withPropsCopy(listComponent,  createMenuOptions(onClick.value())).draw(drawTools, defaultProps);
  },
};
