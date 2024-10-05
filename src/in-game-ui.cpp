#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;
void setTempViewpoint(glm::vec3 position, glm::quat rotation);


bool showSelectionTexture = false;

void setShowSelectionTexture(bool shouldShow){
	showSelectionTexture = shouldShow;
}

std::string ingameUiTextureName(objid id){
	return "gentexture-ingame-ui-texture-test";
}
std::string selectionTextureName(std::string textureName){
	return textureName + "_selection_texture";

}

void createInGamesUiInstance(InGameUi& inGameUi, objid id){
	modassert(inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end(), "id already exists");

	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 1920, 1080, id);
 	setGameObjectTexture(id, showSelectionTexture ? selectionTextureName(texture) : texture);

 	inGameUi.textDisplays[id] = TextDisplay{
 		.textureId = uiTexture,
 		.handlerFns = {},
 		.mouseCoordNdc = glm::vec2(0.f, 0.f),
 		.routerHistory = createHistory(),
 		.uiStateContext = UiStateContext {
 			.uiState = createUiState(),
 		},
 	};
 	inGameUi.textDisplays.at(id).uiStateContext.value().routerHistory = &inGameUi.textDisplays.at(id).routerHistory.value();
 	// initial route
  pushHistory(inGameUi.textDisplays.at(id).routerHistory.value(), { "gamemenu", "elevatorcontrol" }, true);

};

void freeInGameUiInstance(InGameUi& inGameUi, objid id){
	gameapi -> freeTexture(ingameUiTextureName(id), id);
	inGameUi.textDisplays.erase(id);
}

void zoomIntoGameUi(objid id){
	auto rotation = gameapi -> getGameObjectRotation(id, true);
	auto objectPosition = gameapi -> getGameObjectPos(id, true);
  auto uiOffset = getSingleVec3Attr(id, "in-game-ui-offset");
  auto offset = uiOffset.has_value() ? uiOffset.value() : glm::vec3(0.f, 0.f, 0.f);
	auto position = objectPosition + rotation * offset;
	auto viewOrientation = gameapi -> orientationFromPos(position, objectPosition);
	setTempViewpoint(position, viewOrientation);
}

std::optional<objid> getAnyUiInstance(InGameUi& inGameUi){
	if (inGameUi.textDisplays.size() == 0){
		return std::nullopt;
	}
	auto firstElement = inGameUi.textDisplays.begin() -> first; // will return the first set<int>
	return firstElement;
}


void onInGameUiFrame(UiStateContext& uiState, InGameUi& inGameUi, UiContext& uiContext, std::optional<objid> textureId, glm::vec2 ndiCoord){
	// should make sure the texture id is the same
	bool drawCursor = true;
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		textDisplay.mouseCoordNdc = ndiCoord;

		gameapi -> clearTexture(textDisplay.textureId, std::nullopt, std::nullopt, std::nullopt);
    UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
		textDisplay.handlerFns = handleDrawMainUi(
			actualUiState, 
			uiContext, 
			getGlobalState().selectedId, 
			textDisplay.textureId, 
			drawCursor ? textDisplay.mouseCoordNdc : std::optional<glm::vec2>(std::nullopt)
		);

		auto ndiCoords = uvToNdi(getGlobalState().texCoordUvView);
    gameapi -> idAtCoordAsync(ndiCoords.x, ndiCoords.y, false, textDisplay.textureId, [ndiCoords](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
			modlog("ui pick color on game ui id", std::to_string(selectedId.value()));
			if (selectedId.has_value()){
				modlog("ui pick color on game ui id", std::to_string(selectedId.value()));
				modlog("ui pick color on game ui uv", print(ndiCoords));
			}
    });
	}
}

void onInGameUiMouseClick(UiStateContext& uiState, UiContext& uiContext, InGameUi& inGameUi, objid id, int button, int action, glm::vec2 ndiCoords){
  gameapi -> idAtCoordAsync(ndiCoords.x, ndiCoords.y, false, inGameUi.textDisplays.at(id).textureId, [&uiState, id, ndiCoords, &inGameUi, &uiContext, button, action](std::optional<objid> uiId, glm::vec2 texCoordUv) -> void {
		if (inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end()){
			return;
		}
		auto& handlerFns = inGameUi.textDisplays.at(id).handlerFns; // should probably check this still exists
		if (uiId.has_value()){
			TextDisplay& textDisplay = inGameUi.textDisplays.at(id);
	  	UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
			modlog("ui pick color on game ui id", std::to_string(uiId.value()));
			onMainUiMousePress(actualUiState, uiContext, handlerFns, button, action, uiId.value());
		}
  });
}

void onInGameUiMouseCallback(UiStateContext& uiState, UiContext& uiContext, InGameUi& inGameUi, int button, int action, std::optional<objid> selectedId){
	if (!selectedId.has_value()){
		return;
	}
	auto id = selectedId.value();
	if (inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end()){
		return;
	}
	auto ndiCoords = uvToNdi(getGlobalState().texCoordUvView);

	TextDisplay& textDisplay = inGameUi.textDisplays.at(id);
  UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
	onInGameUiMouseClick(actualUiState, uiContext, inGameUi, id, button, action, ndiCoords);
}

void onInGameUiMouseMoveCallback(InGameUi& inGameUi, double xPos, double yPos, float xNdc, float yNdc){
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		//textDisplay.mouseCoordNdc.x += (xPos / 100.f);
		//textDisplay.mouseCoordNdc.y += (yPos / 100.f);
		modlog("onInGameUiMouseMoveCallback", print(textDisplay.mouseCoordNdc));
	}
}

void onInGameUiScrollCallback(InGameUi& inGameUi, double amount){

}

void onInGameUiKeyCallback(int key, int scancode, int action, int mods){

}

struct WaypointPosition {
	glm::vec3 position;
};
struct WaypointObject {
	objid id;
};
typedef std::variant<WaypointPosition, WaypointObject> WaypointType;
std::unordered_map<objid, WaypointType> waypoints = {};

objid addWaypoint(){
	auto id = getUniqueObjId();
	waypoints[id] = WaypointPosition {
		.position = glm::vec3(0.f, 0.f, 0.f),
	} ;
	return id;
}
void removeWaypoint(objid id){
	waypoints.erase(id);
}

auto _ = addWaypoint();


glm::vec2 positionToNdi(glm::vec3 position);

void drawWaypoints(){
	for (auto &[id, waypoint] : waypoints){
		auto waypointPosition = std::get_if<WaypointPosition>(&waypoint);
		if (waypointPosition){
			auto screenspacePosition = positionToNdi(waypointPosition -> position);
			modlog("waypoint", print(screenspacePosition));

	   	if (screenspacePosition.x > 1.f || screenspacePosition.x < -1.f || screenspacePosition.y > 1.f || screenspacePosition.y < -1.f){
		    gameapi -> drawText("waypoint out of range", 0.f, 0.f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        gameapi -> drawLine2D(glm::vec3(0.f, 0.f, 0.f), glm::vec3(screenspacePosition.x, screenspacePosition.y, 0.f), false, glm::vec4(1.f, 0.f, 0.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
	   	}else{
  	    gameapi -> drawRect(screenspacePosition.x, screenspacePosition.y, 0.02f, 0.02f, false, glm::vec4(0.f, 0.f, 1.f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
			  int distance = 204;
			  gameapi -> drawText(std::to_string(distance), screenspacePosition.x, screenspacePosition.y, 10.f, false, glm::vec4(0.f, 0.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	   	}
			return;
		}
		
		auto waypointObject = std::get_if<WaypointObject>(&waypoint);
		if (waypointObject){
			auto position = gameapi -> getGameObjectPos(waypointObject -> id, true);
		 	drawSphereVecGfx(position, 1.f, glm::vec4(0.f, 0.f, 1.f, 0.8f));
			return;
		}

	}
}