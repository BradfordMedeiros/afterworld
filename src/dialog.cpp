#include "./dialog.h"

extern CustomApiBindings* gameapi;

struct Node {
  std::map<std::string, std::string> transitionNameToNode;
  std::map<std::string, std::string> properties;  
};  

struct NodeTransition {
  std::string nodeFrom;
  std::string transition;
  std::string nodeTo;
};

struct NodeProperty {
  std::string node;
  std::string key;
  std::string value;
};

struct DialogData {
  std::vector<NodeTransition> transitions;
  std::vector<NodeProperty> properties;
};

std::set<std::string> allNodeNames(DialogData& data){
  std::set<std::string> nodes;
  for (auto transitions : data.transitions){
    nodes.insert(transitions.nodeFrom);
    nodes.insert(transitions.nodeTo);
  }
  for (auto property : data.properties){
    nodes.insert(property.node);
  }
  return nodes;
}

std::map<std::string, std::string> allTransitions(DialogData& data, std::string nodename){
  std::map<std::string, std::string> transitions;
  for (auto transition : data.transitions){
    if (transition.nodeFrom == nodename){
      transitions[transition.transition] = transition.nodeTo;
    }
  }
  return transitions;
}

std::map<std::string, std::string> allProperties(DialogData& data, std::string nodename){
  std::map<std::string, std::string> properties;
  for (auto property : data.properties){
    if (property.node == nodename){
      properties[property.key] = property.value;
    }
  }
  return properties;
}


std::vector<NodeTransition> deserializeTransitions(){
  auto query = gameapi -> compileSqlQuery(
    "select nodeFrom, transition, nodeTo from dialog_nodes"
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::vector<NodeTransition> transitions;
  for (auto value : result){
    assert(value.size() == 3);
    transitions.push_back(NodeTransition{
      .nodeFrom = value.at(0),
      .transition = value.at(1),
      .nodeTo = value.at(2),
    });
  }
  return transitions;
}

std::vector<NodeProperty> deserializeProperties(){
  auto query = gameapi -> compileSqlQuery(
    "select node, key, value from dialog_properties"
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::vector<NodeProperty> properties;
  for (auto value : result){
    assert(value.size() == 3);
    properties.push_back(NodeProperty{
      .node = value.at(0),
      .key = value.at(1),
      .value = value.at(2),
    });
  }
  return properties;
}


DialogData deserializeDialogData(){
  DialogData data {
    .transitions = deserializeTransitions(),
    .properties = deserializeProperties(),
  };
  return data;
}

std::map<std::string, Node> createDialogTree(DialogData data){
  std::map<std::string, Node> dialogTree;
  for (auto nodename : allNodeNames(data)){
    assert(dialogTree.find(nodename) == dialogTree.end());
    dialogTree[nodename] = Node {
      .transitionNameToNode = allTransitions(data, nodename),
      .properties = allProperties(data, nodename),
    };
  }
  return dialogTree;
}

std::string getNodeInfo(std::string name, Node& node, std::map<std::string, std::string>& properties){
  std::string valueStr = name + "\n";
  for (auto &[key, value] : properties){
    valueStr = valueStr + " " + key + " - " +  value + "\n" ;
  }
  return valueStr;
}

void dumpDotFormat(std::map<std::string, Node>& dialogTree){
  std::string text = "strict graph {\n";

  std::set<std::string> nodesInGraph;
  for (auto [name, node] : dialogTree){
    for (auto [transitionName, transitionTo] : node.transitionNameToNode){
      text = text + 
        "\"" + getNodeInfo(name, node, dialogTree.at(name).properties) + "\""  + 
        " -- " + 
        "\"" + getNodeInfo(transitionTo, dialogTree.at(transitionTo), dialogTree.at(transitionTo).properties)  + "\"" +
        " [ label=\"" + transitionName + "\" ];" +
        "\n";
      nodesInGraph.insert(name);
      nodesInGraph.insert(transitionTo);
    } 
  }

  for (auto [name, _] : dialogTree){
    if (nodesInGraph.find(name) == nodesInGraph.end()){
       text = text + "\"" + getNodeInfo(name, dialogTree.at(name), dialogTree.at(name).properties) + "\"" + "\n";
    }
  }
  text = text + "}";
  std::cout << text << std::endl;
}


std::optional<std::map<std::string, Node>> dialogTree = std::nullopt;

std::optional<Node*> nodeForTransition(std::string fromNode, std::string transition){
	Node& node = dialogTree.value().at(fromNode);
	for (auto &[transitionName, nodeTo] : node.transitionNameToNode){
		if (transitionName == transition){
			return &dialogTree.value().at(nodeTo);
		}
	}
	return std::nullopt;
}

std::optional<std::string> getChatText(std::string nodename){
	auto node = nodeForTransition(nodename, "talk");
	std::cout << "properties for node: " << nodename << " [ ";
	//if (node.has_value()){
	//	for (auto &property : node.value() -> properties.at("text")){
	//		std::cout << property << " ";
	//	}
	//}else{
	//	std::cout << "<no value>";
	//}
	//std::cout << " ]" << std::endl;

	auto property = node.has_value() ? node.value() -> properties.at("text") : std::optional<std::string>(std::nullopt);
	return property;
}


std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.stringAttributes.find(key) != objAttr.stringAttributes.end()){
    return objAttr.stringAttributes.at(key);
  }
  return std::nullopt;
}

void handleInteract(objid gameObjId){
  auto objAttr =  gameapi -> getGameObjectAttr(gameObjId);
  auto chatNode = getStrAttr(objAttr, "chatnode");
  if (chatNode.has_value()){
    gameapi -> sendNotifyMessage("dialog:talk", chatNode.value());
  }
  auto triggerSwitch = getStrAttr(objAttr, "trigger-switch");
  if (triggerSwitch.has_value()){
    gameapi -> sendNotifyMessage("switch", triggerSwitch.value());
  }
}

float randomNum(){
  return static_cast<float>(rand()) / (static_cast <float> (RAND_MAX));
}

// should work globally but needs lsobj-attr modifications, and probably should create a way to index these
void handleSwitch(std::string switchValue, objid sceneId){ 
  //wall:switch:someswitch
  //wall:switch-recording:somerecording
  auto objectsWithSwitch = gameapi -> getObjectsByAttr("switch", switchValue, sceneId);

  std::cout << "num objects with switch = " << switchValue << ", " << objectsWithSwitch.size() << std::endl;
  for (auto id : objectsWithSwitch){
    std::cout << "handle switch: " << id << std::endl;
    //wall:switch-recording:somerecording
    // supposed to play recording here, setting tint for now to test
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = {
        .vec3 = {},
        .vec4 = {
          { "tint", glm::vec4(randomNum(), randomNum(), randomNum(), 1.f) },
        },
      },
    };
    gameapi -> setGameObjectAttr(id, attr);

  }
}


CScriptBinding dialogBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	if (!dialogTree.has_value()){
		dialogTree = createDialogTree(deserializeDialogData());
		// dumpDotFormat(dialogTree);
	}
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    if (key == "dialog:talk"){
    	auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "dialog:talk value invalid");
   		auto chatText = getChatText(*strValue);
   		gameapi -> sendNotifyMessage("alert", "dialog chat: " + (chatText.has_value() ? chatText.value() : ("error: no chat text available for node: " + *strValue) ));
    }else if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
    	auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "selected value invalid");
      auto gameObjId = std::atoi(strValue -> c_str());
      handleInteract(gameObjId);
    }else if (key == "switch"){ // should be moved
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "switch value invalid");
      handleSwitch(*strValue, gameapi -> listSceneId(id));
    }
  };
	return binding;
}


