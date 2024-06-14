#include "./inventory.h"

extern CustomApiBindings* gameapi;


std::unordered_map<std::string, float> loadInventory(){
  return {};
}

std::unordered_map<std::string, float> inventoryCount {
  { "gold",  100 },
  { "pistol",  30 },
  { "scrapgun",  5 },
  { "fork",  100 },
  { "pistol-ammo", 50 },
  { "fork-ammo", 50 },
  { "electrogun",  100 },
  { "electrogun-ammo", 10000 },
};

int currentItemCount(std::string name){
  return inventoryCount.at(name);
}

void updateItemCount(std::string name, int count){
  inventoryCount[name] = count;
}


///////////////////
// Gun logic

bool hasGun(std::string& gun){
  return inventoryCount.find(gun) != inventoryCount.end();
}

std::string ammoNameForGun(std::string& gun){
  return gun + "-ammo";
}
int ammoForGun(std::string& gun){
  std::cout << "gun is: " << gun << std::endl;
  std::string ammoName = ammoNameForGun(gun);
  if (inventoryCount.find(ammoName) == inventoryCount.end()){
    return 0;
  }
  return static_cast<int>(inventoryCount.at(ammoName));
}


void setGunAmmo(std::string gun, int currentAmmo){
  updateItemCount(ammoNameForGun(gun), currentAmmo);
}

