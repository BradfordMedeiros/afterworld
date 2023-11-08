#include "./activeplayer.h"

extern CustomApiBindings* gameapi;

std::optional<objid> activePlayerId = std::nullopt;
std::optional<objid> tempCameraId = std::nullopt;

std::optional<objid> getActivePlayerId(){
	return activePlayerId;
}

void setCameraOrMakeTemp(objid id){
	if (tempCameraId.has_value()){
		gameapi -> removeObjectById(tempCameraId.value());
	}
	auto name = gameapi -> getGameObjNameForId(id).value();
	auto isCamera = name.at(0) == '>';
	if (isCamera){
		gameapi -> setActiveCamera(id, -1);
	}else{
    GameobjAttributes attr {
      .stringAttributes = { },
      .numAttributes = {},
      .vecAttr = {  .vec3 = { { "position", glm::vec3(0.f, 0.f, 5.f) }},  .vec4 = {} },
    };
    std::string cameraName = std::string(">player-camera-") + uniqueNameSuffix();
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
    tempCameraId = cameraId;
    gameapi -> makeParent(cameraId, id);
    gameapi -> setActiveCamera(cameraId, -1);
	}
}
void setActivePlayer(std::optional<objid> id){
	if (!id.has_value()){
		return;
	}
	if (activePlayerId.has_value()){
	  gameapi -> sendNotifyMessage("ai-activate", activePlayerId.value());
	}
	setCameraOrMakeTemp(id.value());
  gameapi -> sendNotifyMessage("ai-deactivate", id.value());
	activePlayerId = id.value();
	setActiveEntity(id.value());
  gameapi -> sendNotifyMessage("active-player-change", id.value());
}

void setActivePlayerNext(){
	auto id = setNextEntity();
	if (id.has_value()){
		if (activePlayerId.has_value()){
		  gameapi -> sendNotifyMessage("ai-activate", activePlayerId.value());
		}
		setCameraOrMakeTemp(id.value());
	  gameapi -> sendNotifyMessage("ai-deactivate", id.value());
		activePlayerId = id.value();
	  gameapi -> sendNotifyMessage("active-player-change", id.value());
	}else{
		modassert(false, "setActivePlayerNext no next item");
	}
}

void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  float approximateWidth = text.size() * ndiSize;
  gameapi -> drawText(text, ndiOffsetX - (approximateWidth * 0.5f), ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

bool displayGameOver = false;
void onPlayerFrame(){
	if (displayGameOver){
	  gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.1f, 0.1f, 0.1f, 1.f), std::nullopt, true, std::nullopt, "./res/textures/testgradient.png");
		drawCenteredText("GAME OVER", 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
	}
  //printActivePlayer();
}
void onActivePlayerRemoved(objid id){
	if (activePlayerId.has_value() && activePlayerId.value() == id){
		activePlayerId = std::nullopt;


		auto playerScene = gameapi -> listSceneId(id);
		auto playerPos = gameapi -> getGameObjectPos(id, true);
	
		// check if scene exists
		
	
		modlog("active player, create dead camera at: ", print(playerPos));

		auto createdObjId = id;

		displayGameOver = true;
  	gameapi -> schedule(createdObjId, 5000, NULL, [](void*) -> void {
  	 	gameapi -> sendNotifyMessage("game-over", (int)1);
  		displayGameOver = false;
  	});
	}
}

void printActivePlayer(){
	if (activePlayerId.has_value()){
		modlog("activeplayer", std::to_string(activePlayerId.value()));
	}else{
		modlog("activeplayer", "none");
	}
}

