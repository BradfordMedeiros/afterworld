#include "./boot.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

struct ManagedGame {
	std::string name;
    pid_t pid;
};
std::optional<ManagedGame> managedGame;


struct DebugMenu {
	std::string label;
	std::vector<DebugMenu> submenu;
	std::function<void()> fn;
};

void launchGame(){
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork\n";
        return;
    }
    if (pid == 0) {
        execl("../afterworld/run.sh", "../afterworld/run.sh", (char*)NULL);
        std::cerr << "Failed to execute game\n";
        exit(1);
    }

    managedGame = ManagedGame{
        .name = "Test Game",
        .pid = pid,
    };

    std::thread([pid]() {
        int status = 0;
        pid_t result = waitpid(pid, &status, 0);
        if (result == pid) {
            if (WIFEXITED(status)) {
                std::cout << "Game exited with code " << WEXITSTATUS(status) << std::endl;
            } else if (WIFSIGNALED(status)) {
                std::cout << "Game killed by signal " << WTERMSIG(status) << std::endl;
            }
            if (managedGame && managedGame->pid == pid) {
                managedGame.reset();
            }
        }
    }).detach();
}

void killGame(){
    if (!managedGame) {
        return;
    }
    pid_t pid = managedGame -> pid;
    if (kill(pid, SIGTERM) == 0) {
        std::cout << "Sent SIGTERM to game\n";
    }
}


DebugMenu menu {
	.label = "Tomorrow's Bad Arcade",
	.submenu = {
		DebugMenu { 
			.label = "Games",
			.submenu = {
				DebugMenu {
					.label = "ModEngine",
					.submenu = {
						DebugMenu {
							.label = "Soul Delivery",
							.fn = []() -> void {
								launchGame();
							},
						},
						DebugMenu {
							.label = "Invaders",
							.fn = []() -> void {
								std::cout << "game play version 2"  << std::endl;
								killGame();
							},
						},
					}
				},
				DebugMenu {
					.label = "GameTwo",
				},
			},
		},
		DebugMenu { .label = "Settings" },
		DebugMenu { 
			.label = "Reboot", 
			.fn = []() -> void {
				std::cout << "game play reboot" << std::endl;
			},
		},
	},
};


std::vector<int> menuPath {
	0
};

DebugMenu getDebugMenu(){
	auto currMenu = menu.submenu.at(menuPath.at(0));
	for (int i = 1; i < menuPath.size(); i++){
		currMenu = currMenu.submenu.at(menuPath.at(i));
	}
	return currMenu;
}

DebugMenu getDebugMenuParent(){
	auto currMenu = menu;
	for (int i = 0; i < menuPath.size() - 1; i++){
		currMenu = currMenu.submenu.at(menuPath.at(i));
	}
	return currMenu;
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
    		auto currMenu = getDebugMenu();
    		std::cout << "game play  curr menu: " << currMenu.label << std::endl;
    		if (currMenu.submenu.size() > 0){
	    		menuPath.push_back(0);
    		}else{
    			if (currMenu.fn){
    				currMenu.fn();
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
    	if(managedGame.has_value()){
	  		gameapi -> drawText("Running Game", -0.9f, 0.9f, 12, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  		gameapi -> drawText(managedGame.value().name, -0.9f, 0.8f, 12, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    		return;
    	}

  		gameapi -> drawText(menu.label, -0.9f, 0.9f, 12, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		
  		auto topMenu = menu.submenu;
  		int topDepth = 0;

  		bool keepRendering = true;
  		while(keepRendering){
	  		auto selectedItem = menuPath.at(topDepth);
  			keepRendering = false;

  			auto currMenu = topMenu;
  			auto currDepth = topDepth;
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

  				auto tint = isMenu ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f);
  				gameapi -> drawText(prefix + item.label, -0.9f + 0.2 * currDepth, 0.8f - (i * 0.1f) , 12, false, tint, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  				bool isMenuOpen = isSelected && menuPath.size() > (currDepth + 1);
  				if (isMenuOpen){
  					std::cout << "menu is open: " << item.label << std::endl;
  					keepRendering = true;
  					topMenu = currMenu.at(i).submenu;
  					topDepth++;

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