#ifndef MOD_AFTERWORLD_COMPONENTS_NAVBAR
#define MOD_AFTERWORLD_COMPONENTS_NAVBAR

#include "../components/common.h"
#include "../components/basic/list.h"
#include "./style.h"

enum NavbarType { MAIN_EDITOR, GAMEPLAY_EDITOR, EDITOR_EDITOR }; 
NavbarType strToNavbarType(std::string& layout);

extern Component navbarComponent;

#endif

