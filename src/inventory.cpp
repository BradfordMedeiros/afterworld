#include "./inventory.h"

extern CustomApiBindings* gameapi;

std::unordered_map<std::string, std::unordered_map<std::string, float>> scopenameToInventory {
  { "player", {
      { "gold",  100 },
      { "pistol",  30 },
      { "fork",  100 },
      { "pistol-ammo", 50 },
      { "fork-ammo", 50 },
      { "electrogun",  100 },
      { "electrogun-ammo", 10000 },
    }
  }
};


std::unordered_map<std::string, float>& defaultInventory(){
  return scopenameToInventory.at("player");
}

int currentItemCount(std::string name){
  if (defaultInventory().find(name) == defaultInventory().end()){
    return 0;
  }
  return defaultInventory().at(name);
}

void updateItemCount(std::string name, int count){
  defaultInventory()[name] = count;
}


///////////////////
// Gun logic

bool hasGun(std::string& gun){
  return defaultInventory().find(gun) != defaultInventory().end();
}

std::string ammoNameForGun(std::string& gun){
  return gun + "-ammo";
}
int ammoForGun(std::string& gun){
  std::cout << "gun is: " << gun << std::endl;
  std::string ammoName = ammoNameForGun(gun);
  if (defaultInventory().find(ammoName) == defaultInventory().end()){
    return 0;
  }
  return static_cast<int>(defaultInventory().at(ammoName));
}


void setGunAmmo(std::string gun, int currentAmmo){
  updateItemCount(ammoNameForGun(gun), currentAmmo);
}

