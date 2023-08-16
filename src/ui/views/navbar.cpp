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
		.name = "Sound",
		.dock = "Sound",
	},
	NavbarOption {
		.name = "Text",
		.dock = "Text",
	},
	NavbarOption {
		.name = "Scenegraph",
		.dock = "Scenegraph",
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
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = paddingSymbol,      .value = 0.02f },
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
