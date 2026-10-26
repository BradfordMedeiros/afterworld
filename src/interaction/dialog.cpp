#include "./dialog.h"

extern CustomApiBindings* gameapi;


std::optional<std::string> getChatText(std::string nodename){
	return "hello world";
}

void loadDialogTree(){

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