#include "./dialog.h"

extern CustomApiBindings* gameapi;




struct DialogTreeText {
  std::string name;
  std::string value;
};

std::vector<DialogTreeText> dialogs {
  DialogTreeText { .name = "test1/one", .value = "this is test1/one text" },
  DialogTreeText { .name = "test1/two", .value = "this is test1/two text" },
  DialogTreeText { .name = "one", .value = "this is one text" },
};

void loadDialogTree(){

}

std::optional<std::string> getChatText(std::string nodename){
  for (auto& dialog : dialogs){
    if (dialog.name == nodename){
      return dialog.value;
    }
  }
  return std::nullopt;
}

void doDialogMessage(std::string& value){
  auto chatText = getChatText(value);
  gameapi -> sendNotifyMessage("alert", "dialog chat: " + (chatText.has_value() ? chatText.value() : ("error: no chat text available for node: " + value) ));
}

void handleDialogInteract(objid gameObjId){
  auto objAttr = getAttrHandle(gameObjId);
  auto chatNode = getStrAttr(objAttr, "chatnode");
  if (chatNode.has_value()){
    doDialogMessage(chatNode.value());
  }
}