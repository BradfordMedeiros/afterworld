#include "./compile.h"

//////////////////////////
struct RailEntity {
  glm::vec3 position;
  glm::vec3 rotation; // euler angles
  std::string railName;
  int railIndex;
  int railTime;
  std::optional<std::string> visualization;
  std::optional<std::string> railKey;
  std::optional<std::string> interpolate;
};
struct OrbEntity {
  glm::vec3 position;
  glm::vec3 rotation;  // euler angles
  std::string orbName;
  std::string orbUi;
  std::string level;
  std::vector<std::string> conn;
};

std::vector<int> getOrbConnectionIndex(std::vector<OrbEntity>& orbs, int index){
  std::vector<int> connections;
  OrbEntity& orbEntity = orbs.at(index);
  for (int i = 0; i < orbEntity.conn.size(); i++){
    auto name = orbEntity.conn.at(i);
    bool matchedOrbName = false;
    for (int j = 0; j < orbs.size(); j++){
      if (name == orbs.at(j).orbName && orbEntity.orbUi == orbs.at(j).orbUi){
        connections.push_back(j);
        matchedOrbName = true;
        break;
      }
    }
    if (!matchedOrbName){
      modassert(false, std::string("no matching orb name: ") + name);
    }
  }
  return connections;
}

void addTriggerColor(Entity& entity, std::vector<GameobjAttributeOpts>& attributes){
  auto triggercolor = getValue(entity, "triggercolor");
  if (triggercolor.has_value()){
    attributes.push_back(GameobjAttributeOpts {
     .field = "triggercolor",
     .attributeValue = *triggercolor.value(),
    });  

    auto activeColor = getVec4Value(entity, "activecolor");
    auto unactiveColor = getVec4Value(entity, "unactivecolor");
    if (activeColor.has_value()){
      attributes.push_back(GameobjAttributeOpts {
       .field = "activecolor",
       .attributeValue = activeColor.value(),
      });  
    }
    if (unactiveColor.has_value()){
      attributes.push_back(GameobjAttributeOpts {
       .field = "unactivecolor",
       .attributeValue = unactiveColor.value(),
      });  
    }
  }
}

void addCoreTrench(Entity& entity, std::vector<GameobjAttributeOpts>& attributes, std::string meshpath){
  attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
    .field = "mesh",
    .attributeValue = meshpath,
  });
  attributes.push_back(GameobjAttributeOpts {
    .field = "physics_shape",
    .attributeValue = "shape_exact",
  });
  attributes.push_back(GameobjAttributeOpts {
    .field = "physics",
    .attributeValue = "enabled",
  });

  addTriggerColor(entity, attributes);
}

void addRotation(Entity& entity, std::vector<GameobjAttributeOpts>& attributes){
  // this is wrong
  auto rotationEuler = getVec3Value(entity, "angles");
  auto rotationAngles = rotationEuler.has_value() ? rotationEuler.value() : glm::vec3(0.f, 0.f, 0.f);
  auto rotation = quatFromTrenchBroomAngles(
    rotationAngles.x,
    rotationAngles.y,
    rotationAngles.z
  );
  auto vecValue = serializeQuatToVec4(rotation);
  attributes.push_back(GameobjAttributeOpts {
    .field = "rotation",
    .attributeValue = vecValue,
  });
}

glm::vec4 addSimpleActivatable(bool* _shouldWrite, Entity& entity, std::vector<GameobjAttributeOpts>& attributes, std::string mesh, std::optional<std::string> activationSubmodel, std::vector<std::string> submodelPhysics){
  *_shouldWrite = true;

  attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
    .field = "mesh",
    .attributeValue = mesh,
  });

  attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
    .field = "activatable",
    .attributeValue = "true",
    .submodel = activationSubmodel,
  });

  glm::vec4 tint(1.f, 1.f, 1.f, 1.f);


  auto soulTypePtr = getValue(entity, "type");

  if (soulTypePtr){
    auto soulType = *soulTypePtr.value();
    static std::vector<std::string> validTypes {
      "red", "blue", "yellow", "purple",
    };

    bool validSoulType = false;
    for (auto& validType : validTypes){
      if (soulType == validType){
        validSoulType = true;
        break;
      }
    }
    modassert(validSoulType, "invalid soul type on tube exit");
    int activateMask = 0;
    if (soulType == "red"){
      activateMask = 0b1110;
      tint = glm::vec4(1.f, 0.f, 0.f, 1.f);
    }else if (soulType == "yellow"){
      activateMask = 0b1101;
      tint = glm::vec4(1.f, 1.f, 0.f, 1.f);
    }else if (soulType == "blue"){
      activateMask = 0b1011;
      tint = glm::vec4(0.f, 0.f, 1.f, 1.f);
    }else if (soulType == "purple"){
      activateMask = 0b0111;
      tint = glm::vec4(1.f, 1.f, 0.f, 1.f);
    }

    attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
      .field =  "activate-mask",
      .attributeValue = static_cast<float>(activateMask),
      .submodel = activationSubmodel,
    });
  }

  auto autoreset = getIntValue(entity, "autoreset");
  if (autoreset.has_value()){
    attributes.push_back(GameobjAttributeOpts {
      .field = "activate-autoreset",
      .attributeValue =  static_cast<float>(autoreset.value()),
      .submodel = activationSubmodel,
    }); 
  }

  auto methodPtr = getValue(entity, "method");
  if (methodPtr.has_value()){
    modassert(*methodPtr.value() == "touch" || *methodPtr.value() == "near" || *methodPtr.value() == "trigger", "invalid activate type");
    attributes.push_back(GameobjAttributeOpts {
      .field = "activate-type",
      .attributeValue = *methodPtr.value(),
      .submodel = activationSubmodel,
    });

    if (*methodPtr.value() == "near"){
      auto radius = getFloatValue(entity, "radius");
      if (radius.has_value()){
        attributes.push_back(GameobjAttributeOpts {
          .field = "radius",
          .attributeValue = radius.value(),
          .submodel = activationSubmodel,
        });        
      }
    }

    if (*methodPtr.value() == "touch"){
      auto delay = getIntValue(entity, "delay");
      if (delay.has_value()){
        attributes.push_back(GameobjAttributeOpts {
          .field = "activate-delay",
          .attributeValue = static_cast<float>(delay.value()),
          .submodel = activationSubmodel,
        }); 
      }
    }
  }

  auto activateName = getValue(entity, "name");
  if (activateName.has_value()){
    attributes.push_back(GameobjAttributeOpts {
      .field = "activate-name",
      .attributeValue = *activateName.value(),
      .submodel = activationSubmodel,
    });
  }


  for (auto& submodelPhysic : submodelPhysics){
    attributes.push_back(GameobjAttributeOpts {
      .field = "physics_shape",
      .attributeValue = "shape_exact",
      .submodel = submodelPhysic,
    });
    attributes.push_back(GameobjAttributeOpts {
      .field = "physics",
      .attributeValue = "enabled",
      .submodel = submodelPhysic,
    });
  }

  return tint;
}

std::string getBallGameTemplate(std::string mapFile, std::optional<std::string> templateFile){
  std::string templatePath = "../afterworld/scenes/levels/ball.rawscene";
  if (templateFile.has_value()){
    templatePath = templateFile.value();
  }
  std::cout << "Generate Template: " << templatePath << std::endl;
  return templatePath;
}


struct BallGameCompile {
  std::vector<RailEntity> rails;
  std::vector<OrbEntity> orbs;
};

CompileMapFns getCompileMapForBallGame(){
  auto ballGameCompileSharedPtr = std::make_shared<BallGameCompile>();
  auto compileFn = [ballGameCompileSharedPtr](std::string& brushFileOut, MapData& mapData, Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* modelName, std::vector<AdditionalEntity>& additionalEntities) -> void {
    auto& ballGameCompile = *ballGameCompileSharedPtr;
    auto origin = getValue(entity, "origin");
    auto className = getValue(entity, "classname");


    modassert(className.has_value(), std::string("no className index = ") + std::to_string(entity.index));
    std::cout << "compile index: " << entity.index << std::endl;
    std::cout << "origin: " << (origin.has_value() ? *origin.value() : "no origin") << std::endl;

    addRotation(entity, attributes);
  
    int layerIndex = -1;
    if (isLayerEntity(entity, &layerIndex) && entity.brushes.size() > 0){
      // same as world spawn, but without added keys
      *shouldWrite = true;

      addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".map");

    }else if (*className.value() == "player_start"){
      *modelName = "playerspawn";
      *shouldWrite = true;
    }else if (*className.value() == "spawn_pipe"){
      *shouldWrite = true;
  
      addCoreTrench(entity, attributes, "../gameresources/build/uncategorized/darkwires4.gltf");

    }else if (*className.value() == "activateable"){
      *shouldWrite = true;
      attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
        .field = "mesh",
        .attributeValue = paths::MUSHROOM,
      });

      attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
        .field = "activatable",
        .attributeValue = "true",
      });

      attributes.push_back(GameobjAttributeOpts {
        .field = "scale",
        .attributeValue = glm::vec3(3.f, 3.f, 3.f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_shape",
        .attributeValue = "shape_exact",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics",
        .attributeValue = "enabled",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "activate-type",
        .attributeValue = "trigger",
      });

      attributes.push_back(GameobjAttributeOpts {
        .field = "layer",
        .attributeValue = "nolighting",
      });

      auto targetName = getValue(entity, "target");
      modassert(targetName.has_value(), "activatable but does not have a target to activate");
      attributes.push_back(GameobjAttributeOpts {
        .field = "activate-target",
        .attributeValue = *targetName.value(),
      });

    }else if (*className.value() == "soul"){
      *shouldWrite = true;

      auto originalModelName = *modelName;
      *modelName = std::string("+") + *modelName;

      attributes.push_back(GameobjAttributeOpts {
        .field = "scale",
        .attributeValue = glm::vec3(0.5f, 0.5f, 0.5f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "state",
        .attributeValue = "enabled",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "effekseer",
        .attributeValue = "./res/particles/spirit-white.efkefc",
      });


      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_shape",
        .attributeValue = "shape_sphere",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics",
        .attributeValue = "enabled",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_collision",
        .attributeValue = "nocollide",
      });



      auto soulTypePtr = getValue(entity, "type");
      modassert(soulTypePtr.has_value(), "soul needs a type field");

      auto soulType = *soulTypePtr.value();
      static std::vector<std::string> validTypes {
        "red", "blue", "yellow", "purple",
      };

      bool validSoulType = false;
      for (auto& validType : validTypes){
        if (soulType == validType){
          validSoulType = true;
          break;
        }
      }
      modassert(validSoulType, std::string("invalid soul type: ") + soulType);
      attributes.push_back(GameobjAttributeOpts {
        .field = "soul",
        .attributeValue = soulType,
      });


      glm::vec4 color(1.f, 1.f, 1.f, 1.f);
      if (soulType == "red"){
        color = glm::vec4(1.f, 0.f, 0.f, 1.f);
      }else if (soulType == "blue"){
        color = glm::vec4(0.f, 0.f, 1.f, 1.f);
      }else if (soulType == "yellow"){
        color = glm::vec4(1.f, 1.f, 0.f, 1.f);
      }else if (soulType == "purple"){
        color = glm::vec4(1.f, 0.f, 1.f, 1.f);
      }

      attributes.push_back(GameobjAttributeOpts {
        .field = "effect-tint",
        .attributeValue = color,
      });


      {
        std::vector<GameobjAttributeOpts> attributes;
        attributes.push_back(GameobjAttributeOpts {
          .field = "mesh",
          .attributeValue = paths::SOUL_HOLDER,
        });

        auto position = getEntityPosition(mapData, entity);
        attributes.push_back(GameobjAttributeOpts {
          .field = "position",
          .attributeValue = position + glm::vec3(0.f, -0.72f, 0.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });
        additionalEntities.push_back(AdditionalEntity {
          .modelName = originalModelName + "_holder",
          .attributes = attributes,
        });
      }

    }else if (*className.value() == "shard"){
      *shouldWrite = true;
      addCoreTrench(entity, attributes, "../gameresources/build/primitives/sphere.gltf");
      attributes.push_back(GameobjAttributeOpts {
        .field = "scrollspeed",
        .attributeValue = glm::vec3(1.f, 1.f, 0.f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "scrollspeed",
        .attributeValue = glm::vec3(1.f, 1.f, 0.f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "breakable",
        .attributeValue = "true",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_collision",
        .attributeValue = "nocollide",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "tint",
        .attributeValue = glm::vec4(0.f, 0.f, 1.f, 0.8f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "layer",
        .attributeValue = "nolighting",
      });

    }else if (*className.value() == "powerup_jump" || *className.value() == "powerup_dash" || *className.value() == "powerup_teleport" || *className.value() == "powerup_lowgravity" || *className.value() == "powerup_invincibility"){
      *shouldWrite = true;

      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_shape",
        .attributeValue = "shape_sphere",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics",
        .attributeValue = "enabled",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_collision",
        .attributeValue = "nocollide",
      });

      // When it becomes deactivated (respawnable but used) make it transparent
      // So hence this layer
      attributes.push_back(GameobjAttributeOpts {
        .field = "layer",
        .attributeValue = "transparency",
      });

      attributes.push_back(GameobjAttributeOpts {
        .field = "spin",
        .attributeValue = "true",
      });

      auto rate = getIntValue(entity, "rate");
      if (rate.has_value()){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup-rate",
          .attributeValue = static_cast<float>(rate.value()),
        });
      }

      if (*className.value() == "powerup_jump"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "jump",
        });
        attributes.push_back(GameobjAttributeOpts {  
          .field = "tint",
          .attributeValue = glm::vec4(1.f, 0.f, 0.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_JUMP,
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_JUMP,
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "texture",
          .attributeValue = paths::INVADERS_SHIP,
        });
      }else if (*className.value() == "powerup_dash"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "dash",
        });
        attributes.push_back(GameobjAttributeOpts {  
          .field = "tint",
          .attributeValue = glm::vec4(0.f, 0.f, 1.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_DASH,
        });
      }else if (*className.value() == "powerup_teleport"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "low_gravity",
        });
        attributes.push_back(GameobjAttributeOpts {  
          .field = "tint",
          .attributeValue = glm::vec4(0.f, 1.f, 1.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_TELEPORT,
        });
      }else if (*className.value() == "powerup_lowgravity"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "teleport",
        });
        attributes.push_back(GameobjAttributeOpts {  
          .field = "tint",
          .attributeValue = glm::vec4(1.f, 0.f, 1.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_TELEPORT,
        });
      }else if (*className.value() == "powerup_invincibility"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "invincibility",
        });
        attributes.push_back(GameobjAttributeOpts {  
          .field = "tint",
          .attributeValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::POWERUP_MODEL_INVINCIBILITY,
        });
      }else{
        modassert(false, "invalid powerup type");
      }

    }else if (*className.value() == "vertical_bound_plane"){
      *shouldWrite = true;

      double yValueSum = 0;
      int totalPoints = 0;

      for (auto& brush : entity.brushes){
        for (auto &brushFace : brush.brushFaces){
          yValueSum += brushFace.point1.y;
          yValueSum += brushFace.point2.y;
          yValueSum += brushFace.point3.y;
          totalPoints += 3;
        }
      }
        
      modassert(totalPoints > 0, "invalid ballplane no faces");
      auto average = yValueSum / totalPoints;
      modassert(average > -10000 && average < 10000, "invalid ballplane"); // arbitrary numbers to guard against weird

      attributes.push_back(GameobjAttributeOpts {
        .field = "ballplane",
        .attributeValue = "true",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "position", 
        .attributeValue = glm::vec3(0.f, average, 0.f),
      });
    }else if (*className.value() == "worldspawn"){
      *shouldWrite = true;

      addCoreTrench(entity, attributes, brushFileOut);
    }else if (*className.value() == "player_end"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".map",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "player_end",
          .attributeValue = "true",
        });
    }else if (*className.value() == "tube_exit"){
        auto tint = addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/uncategorized/darkwires_spawn.gltf", std::nullopt, { "sphere", "model", "spikes", });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field =  "tint",
          .attributeValue = tint,
          .submodel = "sphere",
        });

    }else if (*className.value() == "spikes"){
      addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/misc/spikes_5x5.gltf", std::nullopt, { "model", "spiketip" });
      attributes.push_back(GameobjAttributeOpts {
        .field = "killplane",
        .attributeValue = "true",
        .submodel = "spiketip",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "scale",
        .attributeValue = glm::vec3(3.f, 3.f, 3.f),
      });
    }else if (*className.value() == "crusher"){
      addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/misc/crusher.gltf", std::nullopt,  { "model", "poles" });
      //attributes.push_back(GameobjAttributeOpts {
      //  .field = "killplane",
      //  .attributeValue = "true",
      //  .submodel = "spiketip",
      //});
      //attributes.push_back(GameobjAttributeOpts {
      //  .field = "scale",
      //  .attributeValue = glm::vec3(3.f, 3.f, 3.f),
      //});
    }else if (*className.value() == "spinner"){
      auto spin = getFloatValue(entity, "speed");
      addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/misc/spinner.gltf", std::nullopt, {});
      attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
        .field =  "spin",
        .attributeValue = spin.has_value() ? spin.value() : 1.f,
      });
    }else if (*className.value() == "autodoor"){
      addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/building/autodoor.gltf", std::nullopt, {});
    }else if (*className.value() == "dropper"){
      addSimpleActivatable(shouldWrite, entity, attributes, "../gameresources/build/misc/shootingtarget.gltf", "model", { "model" });
    }else if (*className.value() == "trigger_zone"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".map",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_box",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        auto triggerTarget = getValue(entity, "trigger");
        if (triggerTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_zone",
            .attributeValue = *triggerTarget.value(),
          });
        }

        auto triggerData = getValue(entity, "trigger_data");
        if (triggerData.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_data",
            .attributeValue = *triggerData.value(),
          });
        }
        addTriggerColor(entity, attributes);

        auto triggerEvent = getValue(entity, "trigger_event");
        if (triggerEvent.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_event",
            .attributeValue = *triggerEvent.value(),
          });     
        }

        auto triggerEventValue = getValue(entity, "trigger_event_value");
        if (triggerEventValue.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_event_value",
            .attributeValue = *triggerEventValue.value(),
          });     
        }


        auto cameraTarget = getValue(entity, "camera");
        if (cameraTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "camera_target",
            .attributeValue = *cameraTarget.value(),
          });      
        }

        auto worldSelect = getValue(entity, "worldselect");
        if (worldSelect.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "worldselect",
            .attributeValue = *worldSelect.value(),
          });      
        }

        attributes.push_back(GameobjAttributeOpts {
          .field = "trigger_bound",
          .attributeValue = "true",
        });   
        
    }else if (*className.value() == "lightzone"){
      //modassert(false, "compile for light zone not yet implemented");
      // do nothing, we just reference this in the brush
    }else if (*className.value() == "killplane"){
      *shouldWrite = true;
      addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".map");
      attributes.push_back(GameobjAttributeOpts {
        .field = "killplane",
        .attributeValue = "true",
      });
    }else if (*className.value() == "laser"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, paths::LASER_MODEL);
        attributes.push_back(GameobjAttributeOpts {
          .field = "tint",
          .attributeValue = glm::vec4(0.f, 1.f, 0.f, 1.f),
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "laser",
          .attributeValue = "true",
        });   


        auto laserLength = getScaledFloatValue(mapData, entity, "length");
        attributes.push_back(GameobjAttributeOpts {
          .field = "laserlength",
          .attributeValue = laserLength.value(),
        });       
    }else if (*className.value() == "gravityhole"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, paths::GRAVITYHOLE_MODEL);


        attributes.push_back(GameobjAttributeOpts {
          .field = "gravityhole",
          .attributeValue = "true",
        });     

        auto wellname = getValue(entity, "name");
        if (wellname.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "wellname",
            .attributeValue = *wellname.value(),
          });        
        }

        auto targetwell = getValue(entity, "target");
        if (targetwell.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "targetwell",
            .attributeValue = *targetwell.value(),
          });        
        }




        auto launch = getVec3Value(entity, "launch");
        if (launch.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "launch",
            .attributeValue = changeCoord(launch.value()),
          });     
        }

        auto holemode = getValue(entity, "mode");
        if (holemode.has_value()){
          modassert(*holemode.value() == "auto", "unsupported mode for gravityhole");
          attributes.push_back(GameobjAttributeOpts {
            .field = "holemode",
            .attributeValue = *holemode.value(),
          }); 
        }
    }else if (*className.value() == "bouncepad"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".map");
        auto magnitude = getIntValue(entity, "mag");
        attributes.push_back(GameobjAttributeOpts {
          .field = "bounce",
          .attributeValue = glm::vec3(0.f, 0.f, -1.f * (magnitude.has_value() ? magnitude.value() : 1)),
        });
    }else if (*className.value() == "conveyer"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".map");
        auto modspeed = getVec3Value(entity, "move");
        attributes.push_back(GameobjAttributeOpts {
          .field = "modspeed",
          .attributeValue = modspeed.value(),
        });
    }else if (*className.value() == "water"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".map",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_box",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });
    }else if (*className.value() == "teleport_zone"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".map",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        auto teleportTarget = getValue(entity, "exit");
        if (teleportTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport_zone",
            .attributeValue = *teleportTarget.value(),
          });
        }
    }else if (*className.value() == "teleport_exit"){
        std::cout << "compile map unrecognized type: " << *className.value() << std::endl;
        *shouldWrite = true;
        auto teleportTarget = getValue(entity, "exit");
        if (teleportTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport",
            .attributeValue = "true",
          });
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport_exit",
            .attributeValue = *teleportTarget.value(),
          });             
        }
    }else if (*className.value() == "dynamic"){
        std::cout << "got dynamic" << std::endl;
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".map",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        auto curve = getValue(entity, "curve");
        modassert(curve.has_value(), "curve does not have a value");
        attributes.push_back(GameobjAttributeOpts {
          .field = "curve",
          .attributeValue = *curve.value(),
        });

        auto trigger = getValue(entity, "trigger");
        if (trigger.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger",
            .attributeValue = *trigger.value(),
          });
        }
    }else if (*className.value() == "rail"){
        auto position = getScaledVec3Value(mapData, entity, "origin");
        if (!position.has_value()){
          modassert(false, "rail - position does not have a value");
        }
        auto rail = getValue(entity, "rail");
        modassert(rail.has_value(), "rail does not have a value");

        auto railKey = getValue(entity, "rail-key");

        auto interpolate = getValue(entity, "interpolate");

        auto railIndex = getIntValue(entity, "rail-index");
        modassert(railIndex.has_value(), "rail-index does not have a value");

        auto railTime = getIntValue(entity, "time");
        auto rotationEuler = getVec3Value(entity, "angles");

        auto railVisualization = getValue(entity, "rail-visual");
        auto railVisualizationStr = railVisualization.has_value() ? *railVisualization.value() : "";

        ballGameCompile.rails.push_back(RailEntity {
          .position = position.value(),
          .rotation = rotationEuler.has_value() ? rotationEuler.value() : glm::vec3(0.f, 0.f, 0.f),
          .railName = *rail.value(),
          .railIndex = railIndex.value(),
          .railTime = railTime.has_value() ? railTime.value() : -1,
          .visualization = railVisualizationStr,
          .railKey = railKey.has_value() ? *railKey.value() : std::optional<std::string>(std::nullopt),
          .interpolate = interpolate.has_value() ? *interpolate.value() : std::optional<std::string>(std::nullopt),
        });
    }else if (*className.value() == "orb"){
        auto position = getScaledVec3Value(mapData, entity, "origin");
        if (!position.has_value()){
          modassert(false, "orb - position does not have a value");
        }

        auto orb = getValue(entity, "orb");
        modassert(orb.has_value(), "orb does not have a value");

        auto orbUi = getValue(entity, "orbui");
        modassert(orbUi.has_value(), "orbUi does not have a value");

        auto orbLevel = getValue(entity, "level");
        modassert(orbLevel.has_value(), "orb does not have a level");

        auto rotationEuler = getVec3Value(entity, "angles");
        auto rotation = rotationEuler.has_value() ? rotationEuler.value() : glm::vec3(0.f, 0.f, 0.f);

        OrbEntity orbEntity {
          .position = position.value(),
          .rotation = rotation,
          .orbName = *orb.value(),
          .orbUi = *orbUi.value(),
          .level = *orbLevel.value(),
          .conn = {},
        };

        auto conn = getValue(entity, "conn");
        modassert(conn.has_value(), "orb - conn does not have a value");

        auto connections = split(*conn.value(), ',');
        orbEntity.conn = connections;
        ballGameCompile.orbs.push_back(orbEntity);
    }else if (*className.value() == "camera"){
        *shouldWrite = true;
        *modelName = std::string(">") + *modelName;

        auto tag = getValue(entity, "tag");
        if (tag.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "cameratag",
            .attributeValue = *tag.value(),
          });
        }

        addRotation(entity, attributes);
        
    }else if (*className.value() == "light"){
      *shouldWrite = true;
      *modelName = std::string("!") + *modelName;
      auto color = getVec3Value(entity, "color");
      attributes.push_back(GameobjAttributeOpts {
        .field = "color",
        .attributeValue = color.has_value() ? color.value() : glm::vec3(1.f, 1.f, 1.f),
      });

      auto type = getValue(entity, "type");
      if (type.has_value()){
        if (*type.value() == "spotlight"){
          attributes.push_back(GameobjAttributeOpts {
            .field = "type",
            .attributeValue = "spotlight",
          });
        }else if (*type.value() == "directional"){
          attributes.push_back(GameobjAttributeOpts {
            .field = "type",
            .attributeValue = "directional",
          });
        }else if (*type.value() == "point"){
          // this is the default do nothing
        }else{
          modassert(false, "invalid light type");
        }
      }

    }else if (*className.value() == "gem"){
        *shouldWrite = true;

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_sphere",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        // Replace with a better gem model
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = paths::GEM_MODEL,
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "tint",
          .attributeValue = glm::vec4(1.f, 0.f, 0.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "layer",
          .attributeValue = "transparency",
        });

        auto gemValue = getValue(entity, "gem");
        modassert(gemValue.has_value(), "gem does not have a value");
        attributes.push_back(GameobjAttributeOpts {
          .field = "gem",
          .attributeValue = *gemValue.value(),
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "condition",
          .attributeValue = "true",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "gem-label",
          .attributeValue = *gemValue.value(),
        });

    }else{
        std::cout << "compile map unrecognized type: " << *className.value() << std::endl;
        *shouldWrite = false;
    }

    auto layer = getValue(entity, "layer");
    if (layer.has_value()){
      bool foundLayer = false;
      for (auto& attribute : attributes){
        if (attribute.field == "layer"){
          foundLayer = true;
          break;
        }
      }
      if (!foundLayer){
        attributes.push_back(GameobjAttributeOpts {
          .field = "layer",
          .attributeValue = *layer.value(),
        });          
      }
    }

    auto scroll = getVec3Value(entity, "scroll");
    if (scroll.has_value()){
      bool foundLayer = false;
      for (auto& attribute : attributes){
        if (attribute.field == "scrollspeed"){
          foundLayer = true;
          break;
        }
      }
      if (!foundLayer){
        attributes.push_back(GameobjAttributeOpts {
          .field = "scrollspeed",
          .attributeValue = scroll.value(),
        });          
      }
    }

  };

  auto finalizeFn = [ballGameCompileSharedPtr](MapData& mapData, std::string& generatedScene) -> void {
    auto& ballGameCompile = *ballGameCompileSharedPtr;
    if (ballGameCompile.rails.size() != 0){
        generatedScene += "combined_entities_rail:rail:true\n";

        {
          std::string data = "combined_entities_rail:data-pos:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + serializeVec(ballGameCompile.rails.at(i).position) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-rot:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + serializeVec(ballGameCompile.rails.at(i).rotation) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }


        {
          std::string data = "combined_entities_rail:data-name:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + ballGameCompile.rails.at(i).railName + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-index:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + std::to_string(ballGameCompile.rails.at(i).railIndex) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-time:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + std::to_string(ballGameCompile.rails.at(i).railTime) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-key:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            std::string value = ballGameCompile.rails.at(i).railKey.has_value() ? ballGameCompile.rails.at(i).railKey.value() : "";
            data = data + value + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }


        {
          std::string data = "combined_entities_rail:data-visual:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            auto visualization = ballGameCompile.rails.at(i).visualization.has_value() ? ballGameCompile.rails.at(i).visualization.value() : "";
            data = data + visualization + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-interp:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            auto interpolate = ballGameCompile.rails.at(i).interpolate.has_value() ? ballGameCompile.rails.at(i).interpolate.value() : "";
            data = data + interpolate + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

    }
    if (ballGameCompile.orbs.size() != 0){
        generatedScene += "combined_entities_orb:orbui:true\n";

        {
          std::string data = "combined_entities_orb:data-pos:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + serializeVec(ballGameCompile.orbs.at(i).position) + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_orb:data-rot:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + serializeVec(ballGameCompile.orbs.at(i).rotation) + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }



        {
          std::string data = "combined_entities_orb:data-conn:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            std::string connections;
            std::vector<int> connectionsIndex = getOrbConnectionIndex(ballGameCompile.orbs, i);
            for (int j = 0; j < connectionsIndex.size(); j++){
              connections += std::to_string(connectionsIndex.at(j));
              if (j != (connectionsIndex.size() - 1)){
                connections += ".";
              }
            }
            data = data + connections + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        //data-name
        {
        std::string data = "combined_entities_orb:data-name:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).orbName + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        //data-orbui
        {
        std::string data = "combined_entities_orb:data-orbui:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).orbUi + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        // data-level
        {
          std::string data = "combined_entities_orb:data-level:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).level + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }
    }
  };

  return CompileMapFns {
    .compileFn = compileFn,
    .finalizeFn = finalizeFn,
    .getTemplateFn = getBallGameTemplate,
  };
}