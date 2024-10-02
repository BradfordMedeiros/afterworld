#include "./inventory.h"

std::unordered_map<objid, std::unordered_map<std::string, float>> scopenameToInventory {};

void addInventory(objid id){
  modlog("add inventory added id:", std::to_string(id));
  modassert(scopenameToInventory.find(id) == scopenameToInventory.end(), "inventory already exists");
  scopenameToInventory[id] = {
    { "gold",  100 },
    { "pistol",  10 },
    { "fork",  100 },
    { "pistol-ammo", 10 },
    { "fork-ammo", 50 },
    { "electrogun",  100 },
    { "electrogun-ammo", 10000 },
  };
}
void removeInventory(objid id){
  if (scopenameToInventory.find(id) != scopenameToInventory.end()){
    modlog("remove inventory added id:", std::to_string(id));
    scopenameToInventory.erase(id);
  }
}


int currentItemCount(objid inventory, std::string name){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "currentItemCount inventory does not exist");
  return scopenameToInventory.at(inventory).at(name);
}

void updateItemCount(objid inventory, std::string name, int count){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "updateItemCount inventory does not exist");
  scopenameToInventory.at(inventory)[name] = count;
}


///////////////////
// Gun logic

bool hasGun(objid inventory, std::string& gun){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "hasGun inventory does not exist");
  return scopenameToInventory.at(inventory).find(gun) != scopenameToInventory.at(inventory).end();
}

std::string ammoNameForGun(std::string& gun){
  return gun + "-ammo";
}
int ammoForGun(objid inventory, std::string& gun){
  std::cout << "gun is: " << gun << std::endl;
  std::string ammoName = ammoNameForGun(gun);
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), std::string("ammoForGun inventory does not exist: ") + std::to_string(inventory));
  return static_cast<int>(scopenameToInventory.at(inventory).at(ammoName));
}


void setGunAmmo(objid inventory, std::string gun, int currentAmmo){
  updateItemCount(inventory, ammoNameForGun(gun), currentAmmo);
}


void debugPrintInventory(){
    const float fontSize = 0.02f;
    drawRightText("inventory\n---------------", -0.9, 0.9f - fontSize, fontSize, std::nullopt, std::nullopt);

    std::vector<objid> inventoryNames;
    for (auto &[name, inventory] : scopenameToInventory){
      inventoryNames.push_back(name);
    }
    int offset = 0;
    for (int i = 0; i < inventoryNames.size(); i++){

      drawRightText(std::to_string(inventoryNames.at(i)), -0.9, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(0.8f, 0.8f, 0.8f, 1.f), std::nullopt);
      offset++;

      for (auto &[name, amount] : scopenameToInventory.at(inventoryNames.at(i))){
        drawRightText(name, -0.8, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
        drawRightText(std::to_string(amount), -0.5, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
        offset++;       
      }
    }
}

