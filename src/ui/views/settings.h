#ifndef MOD_AFTERWORLD_COMPONENTS_SETTINGS
#define MOD_AFTERWORLD_COMPONENTS_SETTINGS

#include "../components/common.h"
#include "../components/basic/layout.h"
#include "../components/basic/listitem.h"
#include "./dock/dock.h"

extern Component settingsComponent;

void initSettings();
void setZoom(float percentage, bool hideGun);

#endif

