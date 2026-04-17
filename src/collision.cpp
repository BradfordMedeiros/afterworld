#include "./collision.h"

extern CustomApiBindings* gameapi;

bool passesSwitchFilter(ObjectAttrHandle& handle, objid otherObjId){
  auto switchFilter = getStrAttr(handle, "switch-filter");
  if (!switchFilter.has_value()){
    return true;
  }
  auto otherAttr = getAttrHandle(otherObjId);
  auto filterValue = getAttr(otherAttr, switchFilter.value().c_str());
  return filterValue.has_value();
}
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey){
  modlog("main collision: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
  auto objAttr1 =  getAttrHandle(obj1);
  std::optional<AttributeValue> switchEnter1 = getAttr(objAttr1, attrForValue.c_str());
  auto switchEnter1Key = getStrAttr(objAttr1, attrForKey.c_str());
  auto switchRemove1 = getStrAttr(objAttr1, "switch-remove");
  auto passedFilter1 = passesSwitchFilter(objAttr1, obj2);
  if (switchEnter1.has_value() && passedFilter1){
    //std::cout << "race publishing 1: " << switchEnter1.value() << std::endl;
    auto key = switchEnter1Key.has_value() ? switchEnter1Key.value() : "switch";
    std::cout << "handle collision: " << key << ", " << print(switchEnter1.value()) << ", remove = " << print(switchRemove1) << std::endl;

    gameapi -> sendNotifyMessage(key, switchEnter1.value());
    if (switchRemove1.has_value() && switchRemove1.value() == removeKey){
      gameapi -> removeByGroupId(obj1);
    }
  }

  auto objAttr2 = getAttrHandle(obj2);
  std::optional<AttributeValue> switchEnter2 = getAttr(objAttr2, attrForValue.c_str());
  auto switchEnter2Key = getStrAttr(objAttr2, attrForKey.c_str());
  auto switchRemove2 = getStrAttr(objAttr2, "switch-remove");
  auto passedFilter2 = passesSwitchFilter(objAttr2, obj1);
  if (switchEnter2.has_value() && passedFilter2){
    //std::cout << "race publishing 2: " << switchEnter2.value() << std::endl;
    auto key = switchEnter2Key.has_value() ? switchEnter2Key.value() : "switch";
    std::cout << "handle collision:2 " << key << ", " << print(switchEnter2.value()) << ", remove = " << print(switchRemove2) << std::endl;
    
    gameapi -> sendNotifyMessage(key, switchEnter2.value());
    if (switchRemove2.has_value() && switchRemove2.value() == removeKey){
      gameapi -> removeByGroupId(obj2);
    }
  }
}





