#include "./inventory.h"

extern std::unordered_map<objid, Inventory> scopenameToInventory;     // static-state extern

const int INFINITE_ITEM_COUNT = 9999;
void addInventory(std::unordered_map<objid, Inventory>& scopenameToInventory, objid id){
  modlog("add inventory added id:", std::to_string(id));
  modassert(scopenameToInventory.find(id) == scopenameToInventory.end(), "inventory already exists");
  scopenameToInventory[id] = {
    Inventory {
      .infinite = true,
      .items = {
        { "gold",  100 },
        { "pistol",  10 },
        { "fork",  100 },
        { "pistol-ammo", 10 },
        { "fork-ammo", 50 },
        { "electrogun",  100 },
        { "electrogun-ammo", 10000 },
      }
    }
  };
}
void removeInventory(std::unordered_map<objid, Inventory>& scopenameToInventory, objid id){
  if (scopenameToInventory.find(id) != scopenameToInventory.end()){
    modlog("remove inventory added id:", std::to_string(id));
    scopenameToInventory.erase(id);
  }
}

void setInventoryInfinite(std::unordered_map<objid, Inventory>& scopenameToInventory, objid inventory, bool infinite){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), std::string("setInventoryInfinite inventory does not exist: ") + std::to_string(inventory));
  scopenameToInventory.at(inventory).infinite = infinite;
}


int currentItemCount(std::unordered_map<objid, Inventory>& scopenameToInventory, objid inventory, std::string name){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "currentItemCount inventory does not exist");
  Inventory& inven = scopenameToInventory.at(inventory);
  if (inven.infinite){
    return INFINITE_ITEM_COUNT;
  }
  return inven.items.at(name);
}

void updateItemCount(std::unordered_map<objid, Inventory>& scopenameToInventory, objid inventory, std::string name, int count){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "updateItemCount inventory does not exist");
  scopenameToInventory.at(inventory).items[name] = count;
}


///////////////////
// Gun logic

bool hasGun(objid inventory, std::string& gun){
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), "hasGun inventory does not exist");
  Inventory& inven = scopenameToInventory.at(inventory);
  if (inven.infinite){
    return true;
  }
  return inven.items.find(gun) != inven.items.end();
}

std::string ammoNameForGun(std::string& gun){
  return gun + "-ammo";
}
int ammoForGun(objid inventory, std::string& gun){
  std::cout << "gun is: " << gun << std::endl;
  std::string ammoName = ammoNameForGun(gun);
  modassert(scopenameToInventory.find(inventory) != scopenameToInventory.end(), std::string("ammoForGun inventory does not exist: ") + std::to_string(inventory));

  Inventory& inven = scopenameToInventory.at(inventory);
  if (inven.infinite){
    return INFINITE_ITEM_COUNT;
  }
  return static_cast<int>(inven.items.at(ammoName));
}

void setGunAmmo(objid inventory, std::string gun, int currentAmmo){
  updateItemCount(scopenameToInventory, inventory, ammoNameForGun(gun), currentAmmo);
}

void debugPrintInventory(std::unordered_map<objid, Inventory>& scopenameToInventory){
    const float fontSize = 0.02f;
    drawRightText("inventory\n---------------", -0.9, 0.9f - fontSize, fontSize, std::nullopt, std::nullopt, std::nullopt);

    std::vector<objid> inventoryNames;
    for (auto &[name, inventory] : scopenameToInventory){
      inventoryNames.push_back(name);
    }
    int offset = 0;
    for (int i = 0; i < inventoryNames.size(); i++){

      drawRightText(std::to_string(inventoryNames.at(i)), -0.9, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(0.8f, 0.8f, 0.8f, 1.f), std::nullopt, std::nullopt);
      offset++;

      for (auto &[name, amount] : scopenameToInventory.at(inventoryNames.at(i)).items){
        drawRightText(name, -0.8, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, std::nullopt);
        drawRightText(std::to_string(amount), -0.5, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, std::nullopt);
        offset++;       
      }
    }
}

