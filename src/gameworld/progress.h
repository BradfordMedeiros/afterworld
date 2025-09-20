#ifndef MOD_AFTERWORLD_PROGRESS
#define MOD_AFTERWORLD_PROGRESS

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"


struct Crystal {
  std::string label;
};
struct CrystalPickup {
  bool hasCrystal;
  Crystal crystal;
};

std::vector<CrystalPickup> loadCrystals();
void saveCrystals();
int numberOfCrystals();
int totalCrystals();
void pickupCrystal(std::string name);
bool hasCrystal(std::string& name);

//////////////

void saveData();

#endif