#include "./navbar.h"

struct NavbarOption {
	const char* name;
};

std::vector<NavbarOption> navbarOptions = {
	NavbarOption {
		.name = "Editor",
	},
	NavbarOption {
		.name = "Object Details",
	},
	NavbarOption {
		.name = "Cameras",
	},
	NavbarOption {
		.name = "Lights",
	},
	NavbarOption {
		.name = "Sound",
	},
	NavbarOption {
		.name = "Text",
	},
	NavbarOption {
		.name = "Scenegraph",
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




Props createMenuOptions(std::function<void(const char*)> onClickNavbar){
  std::vector<ListComponentData> levels;
  for (auto &navbarOption : navbarOptions){
  	levels.push_back(ListComponentData {
  		.name = navbarOption.name,
  		.onClick = [&navbarOption, onClickNavbar]() -> void {
  			onClickNavbar(navbarOption.name);
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
  .draw = withPropsCopy(
  	listComponent, 
  	createMenuOptions(
  		[](const char* name) -> void {
  			std::cout << "navbar on click: " << name << std::endl;
  		})
  ).draw,
};
