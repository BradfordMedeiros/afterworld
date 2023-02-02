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
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {},  .vec4 = {} },
  };
	std::map<std::string, GameobjAttributes> submodelAttributes;

	for (auto &sound : sounds){
		attr.stringAttributes["clip"] = sound.clip;
		gameapi -> makeObjectAttr(sceneId, std::string("&material-") + sound.material, attr, submodelAttributes);
	}

	
}

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
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
  	auto keyValue = split(key, ':');
    if (keyValue.size() == 2 && keyValue.at(0) == "play-material-sound"){
    	Sound* sound = static_cast<Sound*>(data);
      auto material = keyValue.at(1);
      auto clip = getClipForMaterial(*sound, material);
      if (!clip){
      	return;
      }

      auto soundPosition = std::get_if<glm::vec3>(&value);
      modassert(soundPosition != NULL, "sound position not given");
      std::cout << "want to play clip: " << *clip << std::endl;
      gameapi -> playClip(std::string("&material-" + material), gameapi -> listSceneId(id), std::nullopt, *soundPosition); // should add playclip position
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    Sound* value = static_cast<Sound*>(data);
    if (key == 'L'){
    	gameapi -> sendNotifyMessage("play-material-sound:wood", glm::vec3(0.f, 0.f, 1.f));
    }
  };

	return binding;
}