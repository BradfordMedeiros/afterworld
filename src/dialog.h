#ifndef MOD_AFTERWORLD_DIALOG
#define MOD_AFTERWORLD_DIALOG

#include <iostream>
#include <vector>
#include "./util.h"

void loadDialogTree();
void onDialogMessage(std::string& key, std::any& value);

#endif 