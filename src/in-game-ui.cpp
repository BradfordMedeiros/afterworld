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

struct WaypointObject {
	std::optional<objid> id;
	bool drawDistance;
};
std::unordered_map<objid, WaypointObject> waypoints = {};

objid addWaypoint(std::optional<objid> waypointId){
	auto id = getUniqueObjId();
	waypoints[id] = WaypointObject {
		.id = waypointId,
		.drawDistance = true,
	};
	return id;
}

void removeWaypoint(objid id){
	waypoints.erase(id);
}

glm::vec2 pointAtSlope(glm::vec2 screenspacePosition, glm::vec4* color, float sizeNdi){
	float slopeY = screenspacePosition.y;
	float slopeX = screenspacePosition.x;
	float slope = slopeY / slopeX;
	float inverseSlope = slopeX / slopeY;

	float halfWidth = sizeNdi * 0.5f;
	float halfHeight = sizeNdi * 0.5f;

	// left and right side
	if (slope > 0 && slope < 1 && screenspacePosition.x > 0){  // top right on x side
		auto yValue = slope * 1.f;
		*color = glm::vec4(1.f, 0.f, 0.f, 0.5f);
		return glm::vec2(1.f, yValue) - halfWidth;
	}
	if (slope < 0 && slope > -1 && screenspacePosition.x < 0){  // top left on -x side
		auto yValue = slope * -1.f;
		*color = glm::vec4(0.f, 0.f, 1.f, 0.5f);
		return glm::vec2(-1.f, yValue) + halfWidth;
	}
	if (slope > 0 && slope < 1 && screenspacePosition.x < 0){  // bottom left on -x side
		auto yValue = slope * -1.f;
		*color = glm::vec4(0.f, 1.f, 0.f, 0.5f);
		return glm::vec2(-1.f, yValue) + halfWidth;
	}
	if (slope < 0 && slope > -1 && screenspacePosition.x > 0){  // bottom right on x side
		auto yValue = slope * 1.f;
		*color = glm::vec4(0.f, 0.f, 1.f, 0.5f);
		return glm::vec2(1.f, yValue) - halfWidth;
	}

	// top and bottom side
	if (slope > 1 && screenspacePosition.x > 0){  // upper right side
		auto xValue = inverseSlope * 1.f;
		*color = glm::vec4(1.f, 0.f, 0.f, 0.5f);
		return glm::vec2(xValue, 1.f) - halfHeight;
	}
	if (slope > 1 && screenspacePosition.x < 0){  // bottom left side
		auto xValue = inverseSlope * -1.f;
		*color = glm::vec4(1.f, 0.f, 0.f, 0.5f);
		return glm::vec2(xValue, -1.f) + halfHeight;
	}

	if (slope < 1 && screenspacePosition.x > 0){  // bottom right side
		auto xValue = inverseSlope * -1.f;
		*color = glm::vec4(1.f, 0.f, 0.f, 0.5f);
		return glm::vec2(xValue, -1.f) + halfHeight;
	}
	if (slope < 1 && screenspacePosition.x < 0){  // top left side
		auto xValue = inverseSlope * 1.f;
		*color = glm::vec4(1.f, 0.f, 0.f, 0.5f);
		return glm::vec2(xValue, 1.f) - halfHeight;
	}

	*color = glm::vec4(0.f, 0.f, 0.f, 0.f);
	return glm::vec2(0.f, 0.f);
}

void drawWaypoint(glm::vec3 position, glm::vec3 playerPos, bool drawDistance){
	auto ndiPosition = gameapi -> positionToNdi(position);
	// ndi position is basically ndi intersects with screenspace
	// when it's behind us, we could just draw it, and would be accurate. 
	// but instead -1  makes it so you reflect which is path to the object
	// the max length means its guaranteed to fall outside the screen (which would otherwise show on the opposite side)
	if (ndiPosition.z < 0){ 
		auto maxLength = glm::max(1 / glm::abs(ndiPosition.x), 1 / glm::abs(ndiPosition.y));  
		ndiPosition.x *= -1 * maxLength;
		ndiPosition.y *= -1 * maxLength;
	}
	auto screenspacePosition = glm::vec2(ndiPosition.x, ndiPosition.y);
	modlog("waypoint", print(screenspacePosition));
	if (screenspacePosition.x > 1.f || screenspacePosition.x < -1.f || screenspacePosition.y > 1.f || screenspacePosition.y < -1.f){
	  //gameapi -> drawText("waypoint out of range", 0.f, 0.f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    //gameapi -> drawLine2D(glm::vec3(0.f, 0.f, 0.f), glm::vec3(screenspacePosition.x, screenspacePosition.y, 0.f), false, glm::vec4(1.f, 0.f, 0.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
		glm::vec4 color(1.f, 1.f, 1.f, 1.f);
		auto point = pointAtSlope(screenspacePosition, &color, 0.02f);
    gameapi -> drawRect(point.x, point.y, 0.02f, 0.02f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
	}else{
  	gameapi -> drawRect(screenspacePosition.x, screenspacePosition.y, 0.02f, 0.02f, false, glm::vec4(0.f, 0.f, 1.f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
		if (drawDistance){
			int distance = glm::distance(position, playerPos);
			gameapi -> drawText(std::to_string(distance), screenspacePosition.x, screenspacePosition.y, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);	   	
		}
	}
}
void drawWaypoints(glm::vec3 playerPos){
	for (auto &[id, waypoint] : waypoints){
		if (waypoint.id.has_value()){
			auto objectExists = gameapi -> gameobjExists(waypoint.id.value());
			if (objectExists){
				auto position = gameapi -> getGameObjectPos(waypoint.id.value(), true);
				drawWaypoint(position, playerPos, waypoint.drawDistance);
			}
			return;
		}
	}
}