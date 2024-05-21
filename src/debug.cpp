#include "./debug.h"

extern CustomApiBindings* gameapi;

void spawnProcMesh(objid sceneId){
	std::vector<glm::vec3> faces = {
		{ glm::vec3(0.f, 0.f, 0.f) },
		{ glm::vec3(1.f, 0.f, 0.f) },
		{ glm::vec3(0.f, 1.f, 0.f) },
	};
	std::vector<glm::vec3> points = {
		glm::vec3(0.f, 0.f, 0.f),
	};
	gameapi -> generateMesh(faces, points, "proc-sometest-mesh");

  GameobjAttributes attr {
    .attr = {
			{ "mesh", "proc-sometest-mesh" },
			{ "position", glm::vec3(0.f, 0.f, 0.f) },
	  	{ "scale", glm::vec3(1.f, 1.f, 1.f) },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(sceneId, "generatedMesh", attr, submodelAttributes);
}

struct PrintObjDebug {
	std::string objname;
	std::string attribute;
	float interval;
};

std::optional<PrintObjDebug> getPrintObjDebug(std::map<std::string, std::string>& args){
	if (args.find("printdebug") == args.end()){
		return std::nullopt;
	}
	auto values = split(args.at("printdebug"), ':');
	auto printRateStr = args.find("printrate") == args.end() ? "1000" : args.at("printrate");
	modassert(values.size() == 2, "invalid value for printdebug");
	return PrintObjDebug {
		.objname = values.at(0),
		.attribute = values.at(1),
		.interval = parseFloat(printRateStr) / 1000.f,
	};
}

CScriptBinding debugBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	auto args = api.getArgs();
	auto printKey = args.find("printkey") != args.end();
	auto printObjDebug = getPrintObjDebug(args);
  binding.onKeyCallback = [printKey](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
   	if (printKey){
   		std::cout << "debugBinding: key = " << key << ", action == " << action << ", scancode = " << scancode << ", mods = " << mods << std::endl;
   	}
    std::cout << "key is: " << key << std::endl;
 
    if (key == 'M' && action == 0){
      spawnFromRandomSpawnpoint("red");
    }else if (key == ',' && action == 0){
      spawnFromAllSpawnpoints("red");
    }else if (key == '.' && action == 0){
      spawnFromAllSpawnpoints("blue");
    }else if (key == '/' && action == 0){
      removeAllSpawnedEntities();
    }
    
   	if (key == 75){
   		//spawnProcMesh(gameapi -> listSceneId(id));
   	}
   	if (key == 96 /* ~ */  && action == 1){
   		getGlobalState().showConsole = !getGlobalState().showConsole;
   		modlog("console visibility", print(getGlobalState().showConsole));
   	}

   	auto testObject = findObjByShortName("testobject");
   	if (testObject.has_value()){
	   	if (key == 340 /* shift */ && action == 1){
	   		gameapi -> setSingleGameObjectAttr(testObject.value(), "position", glm::vec3(0.f, 2.f, 0.f));
	   		gameapi -> setSingleGameObjectAttr(testObject.value(), "physics_mass", 10.f);
	   		gameapi -> setSingleGameObjectAttr(testObject.value(), "tint", glm::vec4(1.f, 0.f, 0.f, 1.f));
	   		gameapi -> setSingleGameObjectAttr(testObject.value(), "custom_string", "cool gamer");
	   	}
   		if (key == 340 /* shift */ && action == 1){
	   		//gameapi -> setSingleGameObjectAttr(testObject.value(), "tint", glm::vec4(0.f, 1.f, 0.f, 1.f));
	   	}


   		std::optional<glm::vec4*> tintValue = getTypeFromAttr<glm::vec4>(getObjectAttributePtr(testObject.value(), "tint"));
   		if (tintValue.has_value()){
	   		std::cout << "tintValue: " << print(*tintValue.value()) << std::endl;
   		}
   		std::optional<glm::vec3*> positionValue = getTypeFromAttr<glm::vec3>(getObjectAttributePtr(testObject.value(), "position"));
   		if (positionValue.has_value()){
	   		std::cout << "positionValue: " << print(*positionValue.value()) << std::endl;
   		}
   		std::optional<std::string*> layerValue = getTypeFromAttr<std::string>(getObjectAttributePtr(testObject.value(), "layer"));
   		if (layerValue.has_value()){
	   		std::cout << "layerValue: " << print(*layerValue.value()) << std::endl;
   		}

   		
   		std::optional<bool*> physicsEnabledValue = getTypeFromAttr<bool>(getObjectAttributePtr(testObject.value(), "physics"));
   		if (physicsEnabledValue.has_value()){
	   		std::cout << "physicsEnabledValue: " << print(*physicsEnabledValue.value()) << std::endl;
   		}

   		//auto objAttr = gameapi -> getGameObjectAttr(testObject.value());
   		//std::cout << "Value objAttr: " << print(objAttr) << std::endl;

   		std::optional<float*> massValue = getTypeFromAttr<float>(getObjectAttributePtr(testObject.value(), "physics_mass"));
   		if (massValue.has_value()){
	   		std::cout << "massValue: " << *(massValue.value()) << std::endl;
   		}

   		std::optional<std::string*> customStrValue = getTypeFromAttr<std::string>(getObjectAttributePtr(testObject.value(), "custom_string"));
   		if (customStrValue.has_value()){
	   		std::cout << "customStrValue: " << *(customStrValue.value()) << std::endl;
   		}else{
	   		std::cout << "customStrValue: " << "none" << std::endl;
   		}

   	}
  };

  if (printObjDebug.has_value()){
  	binding.onFrame = [printObjDebug](int32_t id, void* data) -> void {
  		static float lastPrintTime = 0;
  		auto currTime = gameapi -> timeSeconds(true);
  		if (currTime - lastPrintTime < printObjDebug.value().interval){
  			return;
  		}
 			lastPrintTime = currTime;
  		auto objid = findObjByShortName(printObjDebug.value().objname);
  		if (objid.has_value()){
			  auto objAttr = getAttrHandle(objid.value());
			  auto attr = getAttr(objAttr, printObjDebug.value().attribute.c_str());
			  if (!attr.has_value()){
				  modlog("debug attribute", "no value");
			  }else{
				  modlog("debug attribute", print(attr.value()));
			  }
  		}else{
				  modlog("debug attribute", "no gameobj");
  		}
  	};
  }

	return binding;
}