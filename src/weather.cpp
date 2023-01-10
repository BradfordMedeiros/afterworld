#include "./weather.h"

extern CustomApiBindings* gameapi;

struct Weather {
	std::optional<objid> weatherEmitter;
};

CScriptBinding weatherBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weather* weather = new Weather;

    std::map<std::string, GameobjAttributes> submodelAttributes;

 	  GameobjAttributes particleAttr {
    	.stringAttributes = { 
    		{ "state", "enabled" },    
      	{ "+physics", "enabled" },
      	{ "+physics_type", "dynamic" },  
      	{ "+layer", "basicui" },    ///////
      	{ "+mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf"},
      	{ "+texture", "../gameresources/textures/particles/rain.png" },
    	},
    	.numAttributes = { 
    		{ "duration", 10.f },
    		{ "rate", 0.1f },
    		{ "limit", 100 },
    	},
    	.vecAttr = {  
    		.vec3 = { 
    			{"position", glm::vec3(0, 1.5f, -1.f) }, 
    			{"+scale", glm::vec3(0.2f, 0.2f, 0.2f) },
        	{ "+physics_gravity", glm::vec3(0.f, -1.f, -1.f) },

    			{"!position", glm::vec3(0.001f, 0.001f, 0.001f) },
    			{"?position", glm::vec3(0.1f, 0.001f, 0.001f) },

    		},  
    		.vec4 = {
     			{"+tint", glm::vec4(0.f, 0.f, 5.f, 1.f) },
    		} 
    	},
  	};

    weather -> weatherEmitter = gameapi -> makeObjectAttr(sceneId, "+code-weather", particleAttr, submodelAttributes);

    return weather;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weather* weather = static_cast<Weather*>(data);
    delete weather;
  };

	 return binding;
}