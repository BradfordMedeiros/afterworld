#include "./sound.h"

extern CustomApiBindings* gameapi;

struct MaterialToSound {
	std::string material;
	std::string clip;
};

struct Sound {
	std::vector<MaterialToSound> sounds;
};

std::string* getClipForMaterial(Sound& sound, std::string& material){
	for (auto &sound : sound.sounds){
		if (material == sound.material){
			return &sound.clip;
		}
	}
	return NULL;
}

void loadAllSounds(std::vector<MaterialToSound>& sounds, objid sceneId){
  GameobjAttributes attr {
    .attr = {},
  };
	std::map<std::string, GameobjAttributes> submodelAttributes;

	for (auto &sound : sounds){
		attr.attr["clip"] = sound.clip;
		auto id = gameapi -> makeObjectAttr(sceneId, std::string("&material-") + sound.material, attr, submodelAttributes);
	  modassert(id.has_value(), "could not make material");
  }
}

struct MessagePlaySound {
  glm::vec3 position;
  std::string material;
};


CScriptBinding soundBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
    binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Sound* sound = new Sound;
    auto soundsQuery = gameapi -> compileSqlQuery("select material, walk-sound from materials", {});
    bool validSql = false;
    auto soundsResult = gameapi -> executeSqlQuery(soundsQuery, &validSql);

    std::vector<MaterialToSound> sounds;
    for (auto &soundResult : soundsResult){
    	if (soundResult.at(0) == ""){
    		continue;
    	}
    	sounds.push_back(MaterialToSound{
    		.material = soundResult.at(0),
    		.clip = soundResult.at(1),
    	});
    }

    sound -> sounds = sounds;
    modassert(validSql, "error executing sql query");
		loadAllSounds(sound -> sounds, sceneId);    
    return sound;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Sound* value = static_cast<Sound*>(data);
    delete value;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    if (key == "play-material-sound"){
    	Sound* sound = static_cast<Sound*>(data);
      auto soundPosition = anycast<MessagePlaySound>(value);
      modassert(soundPosition != NULL, "sound position not given");
      auto material = soundPosition -> material;
      auto clip = getClipForMaterial(*sound, material);
      if (!clip){
      	return;
      }
      std::cout << "want to play clip: " << *clip << std::endl;
      playGameplayClip(std::string("&material-" + material), gameapi -> listSceneId(id), std::nullopt, soundPosition -> position); // should add playclip position
    }
  };
  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force) -> void {
    std::cout << "sound: on collision enter: " << obj1 << ", " << obj2 << std::endl;
    Sound* sound = static_cast<Sound*>(data);
    std::string material = "wood";
    auto clip = getClipForMaterial(*sound, material);
    if (clip){
      playGameplayClip(std::string("&material-" + material), gameapi -> listSceneId(id), std::nullopt, pos);
      auto attr1 = getAttrHandle(obj1);
      auto attr2 = getAttrHandle(obj2);

      auto vel1 = getVec3Attr(attr1, "physics_velocity").value();
      auto vel2 = getVec3Attr(attr2, "physics_velocity").value();
      std::cout << "vel1: " << print(vel1) << ", vel2: " << print(vel2) << std::endl;
    }
  };

	return binding;
}