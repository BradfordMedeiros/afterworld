#include "./inventory.h"

extern CustomApiBindings* gameapi;

std::unordered_map<std::string, std::unordered_map<std::string, float>> scopenameToInventory {
    { "default", {
        { "gold",  100 },
        { "pistol",  10 },
        { "fork",  100 },
        { "pistol-ammo", 50 },
        { "fork-ammo", 50 },
        { "electrogun",  100 },
        { "electrogun-ammo", 10000 },
    }},
    { "another", {
        { "gold",  100 },
        { "pistol",  10 },
        { "fork",  100 },
        { "pistol-ammo", 5 },
        { "fork-ammo", 50 },
        { "electrogun",  100 },
        { "electrogun-ammo", 10000 },
    }}
};


int currentItemCount(std::string inventory, std::string name){
  if (scopenameToInventory.at(inventory).find(name) == scopenameToInventory.at(inventory).end()){
    return 0;
  }
  return scopenameToInventory.at(inventory).at(name);
}

void updateItemCount(std::string inventory, std::string name, int count){
  scopenameToInventory.at(inventory)[name] = count;
}


///////////////////
// Gun logic

bool hasGun(std::string inventory, std::string& gun){
  return scopenameToInventory.at(inventory).find(gun) != scopenameToInventory.at(inventory).end();
}

std::string ammoNameForGun(std::string& gun){
  return gun + "-ammo";
}
int ammoForGun(std::string inventory, std::string& gun){
  std::cout << "gun is: " << gun << std::endl;
  std::string ammoName = ammoNameForGun(gun);
  if (scopenameToInventory.at(inventory).find(ammoName) == scopenameToInventory.at(inventory).end()){
    return 0;
  }
  return static_cast<int>(scopenameToInventory.at(inventory).at(ammoName));
}


void setGunAmmo(std::string inventory, std::string gun, int currentAmmo){
  updateItemCount(inventory, ammoNameForGun(gun), currentAmmo);
}


void debugPrintInventory(){
    const float fontSize = 0.02f;
    drawRightText("inventory\n---------------", -0.9, 0.9f - fontSize, fontSize, std::nullopt, std::nullopt);

    std::vector<std::string> inventoryNames;
    for (auto &[name, inventory] : scopenameToInventory){
      inventoryNames.push_back(name);
    }
    int offset = 0;
    for (int i = 0; i < inventoryNames.size(); i++){

      drawRightText(inventoryNames.at(i), -0.9, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(0.8f, 0.8f, 0.8f, 1.f), std::nullopt);
      offset++;

      for (auto &[name, amount] : scopenameToInventory.at(inventoryNames.at(i))){
        drawRightText(name, -0.8, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
        drawRightText(std::to_string(amount), -0.5, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
        offset++;       
      }
    }
}

