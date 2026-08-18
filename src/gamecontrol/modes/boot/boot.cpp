#include "./boot.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

union DebugMenuData {
	bool enabled;
};
struct DebugMenu {
	std::string label;
	std::function<std::string(DebugMenu& self)> displayText;
	std::optional<std::string> image;
	std::vector<DebugMenu> submenu;
	DebugMenuData data;
	std::function<void(DebugMenu& self)> fn;
};

DebugMenu createMenu(){
	auto allGames = platform::listGames();

	std::vector<DebugMenu> games = {};
	for (auto& game : allGames){
		games.push_back(DebugMenu {
			.label = game.displayName,
			.image = "./res/textures/wood.jpg",
			.fn = [](DebugMenu&) -> void {
				platform::startGame(game.name);
			},
		});
	}


	DebugMenu menu {
		.label = "Tomorrow's Bad Arcade",
		.submenu = {
			DebugMenu { 
				.label = "Games",
				.submenu = games,
			},
			DebugMenu { 
				.label = "Settings",
				.submenu = {
					DebugMenu {
						.label = "Mute",
						.displayText = [](DebugMenu& self) -> std::string { 
							return platform::isMuted() ? "[X]" : "[]"; 
						},
						.fn = [](DebugMenu& self) -> void {
							auto isMuted = platform::isMuted();
							platform::setMuted(!isMuted);
						},
					},
					DebugMenu {
						.label = "Volume",
						.displayText = [](DebugMenu& self) -> std::string { 
							auto volume = platform::getVolume();
							return std::to_string(volume); 
						},
						.fn = [](DebugMenu& self) -> void {
							auto volume = platform::getVolume();
							volume += 0.1f;
							if (volume > 1.05f){
								volume = 0.f;
							}
							platform::setVolume(volume);
						},
					},
				}
			},
			DebugMenu { 
				.label = "Hardware",
				.submenu = {
					DebugMenu {
						.label = "LED Enabled",
						.displayText = [](DebugMenu& self) -> std::string { 
								return platform::isLedStateEnabled(0) ? "[X]" : "[ ]";
						},
						.data = DebugMenuData{ .enabled = false },
						.fn = [](DebugMenu& self) -> void {
								self.data.enabled = !self.data.enabled;
								platform::setLedState(0, self.data.enabled);
						},
					},
					DebugMenu {
						.label = "LED Color",
						.data = DebugMenuData{ .enabled = false },
						.fn = [](DebugMenu& self) -> void {
						},
					},
				}
			},
			DebugMenu { 
				.label = "Reboot", 
				.fn = [](DebugMenu&) -> void {
					platform::reboot();
				},
			},
		},
	};
	return menu;
}

DebugMenu menu = createMenu();


std::vector<int> menuPath {
	0
};

DebugMenu& getDebugMenu(){
	auto currMenu = &menu.submenu.at(menuPath.at(0));
	for (int i = 1; i < menuPath.size(); i++){
		currMenu = &currMenu -> submenu.at(menuPath.at(i));
	}
	return *currMenu;
}

DebugMenu& getDebugMenuParent(){
	auto currMenu = &menu;
	for (int i = 0; i < menuPath.size() - 1; i++){
		currMenu = &currMenu -> submenu.at(menuPath.at(i));
	}
	return *currMenu;
}


GameTypeInfo getBootMode(){
  GameTypeInfo ballMode = GameTypeInfo {
    .gametypeName = "boot",
    .createGametype = [](void* data) -> std::any {
      return NULL; 
    },
    .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {
    },
    .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
    	if (key == 'Q' && action == 1){
    		if (menuPath.size() > 1){
	    		menuPath.pop_back();
    		}
    	}
    	if (key == 'E' && action == 1){
    		auto& currMenu = getDebugMenu();
    		std::cout << "game play  curr menu: " << currMenu.label << std::endl;
    		if (currMenu.submenu.size() > 0){
	    		menuPath.push_back(0);
    		}else{
    			if (currMenu.fn){
    				currMenu.fn(currMenu);
    			}else{
		    		std::cout << "game play  no fn to dispatch: " << currMenu.label << std::endl;
    			}
    		}
    	}
    	if (key == 'W' && action == 1){
    		auto currValue = menuPath.at(menuPath.size() - 1);
    		currValue = currValue - 1;
    		if (currValue < 0){
    			currValue = 0;
    		}
    		menuPath.at(menuPath.size() - 1) = currValue;
    	}
    	if (key == 'S' && action == 1){
    		auto currMenu = getDebugMenuParent();
    		auto currValue = menuPath.at(menuPath.size() - 1);
    		currValue = currValue + 1;
    		if (currValue >= currMenu.submenu.size()){
    			currValue = currMenu.submenu.size() - 1;
    		}
    		menuPath.at(menuPath.size() - 1) = currValue;
    	}
    },
    .onFrame = [](std::any& gametype) -> void {
    	if (!platform::isConnected()){
  			gameapi -> drawText("not connected", -0.9f, 0.9f, 12, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    		return;
    	}
    	platform::poll();
    	
  		gameapi -> drawText(menu.label, -0.9f, 0.9f, 12, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		
  		auto topMenu = menu.submenu;
  		int topDepth = 0;

  		bool keepRendering = true;
  		while(keepRendering){
	  		auto selectedItem = menuPath.at(topDepth);
  			keepRendering = false;

  			auto currMenu = topMenu;
  			auto currDepth = topDepth;
  			bool isMaxDepth = currDepth == (menuPath.size() - 1);
  			for (int i = 0; i < currMenu.size(); i++){
  				auto item = currMenu.at(i);
  				std::string prefix;

  				bool isSelected = i == selectedItem;
  				bool isMenu = item.submenu.size() > 0;
  				if (isMenu && isSelected){
  					prefix = "> ";
  				}
  				if (!isMenu && isSelected){
  					prefix = "X ";
  				}

  				if (isMaxDepth){
  					prefix += "| ";
  				}

  				auto tint = isMenu ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f);
				std::string fullText = item.label + (item.displayText ? item.displayText(item) : "");

  				gameapi -> drawText(prefix + fullText, -0.9f + 0.2 * currDepth, 0.8f - (i * 0.1f) , 12, false, tint, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  				bool isMenuOpen = isSelected && menuPath.size() > (currDepth + 1);
  				if (isMenuOpen){
  					std::cout << "menu is open: " << item.label << std::endl;
  					keepRendering = true;
  					topMenu = currMenu.at(i).submenu;
  					topDepth++;
  				}

				static std::string lastImage;

  				if (isSelected && isMaxDepth){
  					if (currMenu.at(i).image.has_value()){
  						ShapeOptions shapeOptions {
  							.zIndex = -1,
  						};

  						auto& image = currMenu.at(i).image.value();
  						static float lastFadeTime = 0.f;
  						if (lastImage != image){
  							lastFadeTime = gameapi -> timeSeconds(true);
  						}

  						auto time = gameapi -> timeSeconds(true) - lastFadeTime;
  						float percentage = time / 0.2f;
  						if (percentage > 1.f){
  							percentage = 1.f;
  						}
  		
  						lastImage = image;
  						gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.4f, 0.4f, 0.4f, 1.f * percentage), std::nullopt, true, std::nullopt, currMenu.at(i).image.value(), shapeOptions);  						
  					}else{
  						lastImage = "";
  					}
  				}
  			}  			
  		}

    },
  };
  return ballMode;
}



void startBootMode(objid sceneId){
	auto bootMode = getBootMode();
	changeGameType(gametypeSystem, bootMode, "boot", NULL);
}

void stopBootMode(){

}