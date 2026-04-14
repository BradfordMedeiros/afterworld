#include "./ball.h"

extern CustomApiBindings* gameapi;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;
extern Movement movement;
extern Weapons weapons;
extern AiData aiData;


void enterVehicleRaw(int playerIndex, objid vehicleId, objid id);
void goToLevel(std::string levelShortName);
bool isReloadKey(int button);
void createExplosion(glm::vec3 position, float outerRadius, float damage);
void handleRemoveKillplaneCollision(objid);

bool inBallMode = false;
std::optional<float> ballStartTime;
std::optional<float> finalBallTime;

void ballStartGameplay(EasyCutscene& cutscene){
  if (initialize(cutscene)){
		setDisablePlayerControl(true, 0);
  }
  if (finalize(cutscene)){
  }

  waitUntil(cutscene, 0, 500);
  run(cutscene, 1, []() -> void {
    showLetterBox("Learning to Roll", 10.f);
  });
  run(cutscene, 2, []() -> void {
  	setDisablePlayerControl(false, 0);
  });
}
void ballEndGameplay(EasyCutscene& cutscene){
	if (initialize(cutscene)){
	  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);
		setDisablePlayerControl(true, 0);
	}
  waitUntil(cutscene, 0, 2000);

  if (finishedThisFrame(cutscene, 0)){
		setShowBallOptions(
				BallComponentOptions {
					.levelComplete = BallLevelComplete{},
				}
		);
  }

	waitFor(cutscene, 1, []() -> bool {
		return getGlobalState().leftMouseDown;
	});

  run(cutscene, 2, []() -> void {
 		setShowBallOptions(std::nullopt);
  	goToLevel("ballselect");
  });
}

void setBallLevelComplete(){
	if (!inBallMode){
		return;
	}
	inBallMode = false;
	std::cout << "set ball level complete: " << activeLevel.value() << std::endl;

	auto timeElapsed = gameapi -> timeSeconds(false) - ballStartTime.value();

	changeGameTypeNone(gametypeSystem);
	markLevelComplete(activeLevel.value(), timeElapsed);
	finalBallTime = timeElapsed;

	commitCrystals();

	playCutscene(ballEndGameplay, std::nullopt);	
}

void createBallObj(objid sceneId, glm::vec3 position){
  GameobjAttributes attr { 
  	.attr = {
  		{ "vehicle", "ball" },
			{ "texture", "./res/textures/wood.jpg" },
			{ "tint", glm::vec4(1.f, 1.f, 1.f, 1.f) },
			{ "physics_restitution", 0.5f },
			{ "mesh", "../gameresources/build/characters/ball.gltf" },
			{ "physics_mass" , 10.f },
			{ "physics_type", "dynamic" },
			{ "physics_shape", "shape_sphere" },
			{ "physics", "enabled" },
			{ "position", position},
			{ "layer", "transparency" },
  	} 
  };
	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
 	auto ball = gameapi -> makeObjectAttr(sceneId, std::string("ball1"), attr, submodelAttributes);  

  bool addParticle = true;

  if (addParticle){
  
  	{
  		modassert(ball.has_value(), "ball was not created");
  		GameobjAttributes emitterAttr { 
  			.attr = {
 					{ "effekseer", "./res/particles/spirit.efkefc" },
 					{ "state", "enabled" },
  			} 
  		};
  		std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  		auto ballSpirit = gameapi -> makeObjectAttr(sceneId, std::string("+ballSpirit"), emitterAttr, submodelAttributesEmitter);
  		modassert(ballSpirit.has_value(), "ballSpirit was not created");
  		gameapi -> makeParent(ballSpirit.value(), ball.value());
		}  	
  }


  {
  	GameobjAttributes emitterAttr { 
  		.attr = {
  			{ "clip", "../gameresources/sound/rolling.ogg" },
 				{ "loop", "true" },
  		} 
  	};
  	std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  	auto ballSound = gameapi -> makeObjectAttr(sceneId, std::string("&ballsoundmove"), emitterAttr, submodelAttributesEmitter);
  	modassert(ballSound.has_value(), "ballSound was not created");

  	gameapi -> makeParent(ballSound.value(), ball.value());
	}

}

std::optional<objid> getBallId(){
	  auto vehicles = getVehicleIds();
 	  modassert(vehicles.size() == 1, "invalid expected vehicle size");
	  return vehicles.at(0);
}

void changeUi(bool showBallUi){
	if (showBallUi){
		setShowBallOptions(
			BallComponentOptions {
				.powerupTexture = "../gameresources/build/textures/ballgame/jump.png",
			}
		);
	}else{
		setShowBallOptions(std::nullopt);
	}
}

void showTimeElapsed(bool shouldShow){
		if (!showBallOptions().has_value() || !ballStartTime.has_value()){
			return;
		}
		auto ballOptions = showBallOptions().value();
		if (!shouldShow){
			ballOptions.elapsedTime = std::nullopt;
		}else{
			ballOptions.elapsedTime = []() -> float {
				if (finalBallTime.has_value()){
					return finalBallTime.value();
				}
				return gameapi -> timeSeconds(false) - ballStartTime.value();
			};
		}
		
		setShowBallOptions(ballOptions);
}

void setPowerupTexture(std::string texture, std::optional<float> startTime, std::optional<float> duration){
	auto ballOptions = showBallOptions();
	if (!ballOptions.has_value()){
		return;
	}
	if (ballOptions.has_value()){
		auto newBallOptions = ballOptions.value();
		newBallOptions.powerupTexture = texture;
		newBallOptions.powerupStartTime = startTime;
		newBallOptions.powerupDuration = duration;
		setShowBallOptions(newBallOptions);
	}
}

std::optional<BallPowerupState> getPowerup(){
	auto activePlayer = getActivePlayerId(0);
	auto vehicleIds = getVehicleIds();
	if (vehicleIds.size() == 0){
		return std::nullopt;
	}

	auto vehicleBall = getVehicleBall(vehicles, vehicleIds.at(0));
	modassert(vehicleBall.has_value(), "get powerup - not a ball");
	return getBallPowerup(*vehicleBall.value());
} 

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   std::optional<objid> ballId;
   bool didLose = false;
   bool shouldReset = false;
   bool didReset = false;
};

GameTypeInfo getBallMode();

void startBallMode(objid sceneId){
  //modassert(false, "gamemode ball not implemented");
  auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, std::nullopt).at(0);
  glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj, true, "[gamelogic] startLevel get player spawnpoint");
    
  // TODO - no reason to actually create the prefab here
  auto prefabId = createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy/player-cheap.rawscene",  position, {});    
  auto playerId = findObjByShortName("maincamera", sceneId);
  modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
  setActivePlayer(movement, weapons, aiData, playerId.value(), 0);

  //////////

	inBallMode = true;
	ballStartTime = gameapi -> timeSeconds(false);

	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	auto playerSpawnPosition = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");

	createBallObj(sceneId, playerSpawnPosition);
	BallModeOptions modeOptions {};

	GameTypeInfo ballMode = getBallMode();
	changeGameType(gametypeSystem, ballMode, "ball", &modeOptions);

	setHudEnabled(false);
}

void endBallMode(){
	inBallMode = false;
	ballStartTime = std::nullopt;
	finalBallTime = std::nullopt;

	setCanExitVehicle(true);
	setShowBallOptions(std::nullopt);
	changeGameTypeNone(gametypeSystem);
	setHudEnabled(true);
}

struct GravityHole {
	objid gravityHoleId;
};
void gravityHoleBall(objid gravityHoleId){
	gameapi -> sendNotifyMessage("ballgravity", GravityHole{
		.gravityHoleId = gravityHoleId,
	});
}

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { "ball", "ballgravity" },
	  .createGametype = [](void* data) -> std::any {
			BallModeOptions* modeOptions = static_cast<BallModeOptions*>(data);
			{
				auto activePlayer = getActivePlayerId(0);
				auto vehicles = getVehicleIds();
				modassert(vehicles.size() == 1, "invalid expected vehicle size");
				modassert(activePlayer.has_value(), "ball set active player no active player");
				std::cout << "vehicles: " << print(vehicles) << "[" << gameapi -> getGameObjNameForId(vehicles.at(0)).value()  << "]" << ", playerid: " << print(activePlayer) << ", [" << gameapi -> getGameObjNameForId(activePlayer.value()).value()  <<  "]" << std::endl;
				std::cout << "vehicles: " << gameapi -> getGameObjNameForId(gameapi -> getActiveCamera(std::nullopt).value()).value()  << std::endl;
				enterVehicleRaw(0, vehicles.at(0), activePlayer.value());
				setCanExitVehicle(false);

				playCutscene(ballStartGameplay, std::nullopt);
			}

		  auto ballId = getBallId().value();
	  	auto pos = gameapi -> getGameObjectPos(ballId, true, "[gamelogic] get ball position for start");
	  	modeOptions -> initialBallPos = pos;
	  	modeOptions -> ballId = ballId;
	  	modeOptions -> didLose = false;
	  	modeOptions -> shouldReset = false;
	  	modeOptions -> didReset = false;

			changeUi(true);
 	   	showTimeElapsed(true);

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	if (event == "ballgravity"){
		  	GravityHole* gravityHole = std::any_cast<GravityHole>(&value);
		  	modassert(gravityHole, "expected type for ballgravity");
	  		auto ballVehicle = getVehicleBall(vehicles, ballMode -> ballId.value());
	  		setBallGravityWell(ballMode -> ballId.value(), *ballVehicle.value(), true, gravityHole -> gravityHoleId);
	  		return false;
	  	}

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball mode: " << event << ", " << *message << std::endl;

	  	if (*message == "reset"){
	  		modassert(ballMode -> initialBallPos.has_value(), "no initial ball position");
	  		ballMode -> shouldReset = true;
	  	}

	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	if(isReloadKey(key) && action == 0){
	  		gameapi -> sendNotifyMessage("ball", std::string("reset"));
	  	}
	  	if (isToggleThirdPersonKey(key) && action == 0){
	  		ballMode -> didLose = true;
	  		auto position = gameapi -> getGameObjectPos(ballMode -> ballId.value(), true, "[gamelogic] - ballIntroOpening pos");
	  		createExplosion(position, 5.f, 0.f);
	  	}
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	std::cout << "ball onframe" << std::endl;
	  	if (ballMode -> ballId.has_value()){
	  		if (isInKillPlane(ballMode -> ballId.value()) && !ballMode -> didLose && !ballMode -> didReset){ // didReset b/c otherwise at same pos
		  		auto ballVehicle = getVehicleBall(vehicles, ballMode -> ballId.value());
	  			if (!(ballVehicle.value() -> powerup.has_value() && ballVehicle.value() -> powerup.value().powerup == INVINCIBILITY && ballVehicle.value() -> powerup.value().useTime.has_value())){
	  				ballMode -> didLose = true;
	  				auto position = gameapi -> getGameObjectPos(ballMode -> ballId.value(), true, "[gamelogic] - ballIntroOpening pos");
	  				createExplosion(position, 5.f, 0.f);
	  				setGameObjectMeshEnabled(ballMode -> ballId.value(), false);
	  				setGameObjectPhysicsEnable(ballMode -> ballId.value(), false);
		  			handleRemoveKillplaneCollision(ballMode -> ballId.value());
	  			}

	  		}

	  		if (ballMode -> didReset){ 
	  			ballMode -> didReset = false;
		  		setGameObjectMeshEnabled(ballMode -> ballId.value(), true);
		  		setGameObjectPhysicsEnable(ballMode -> ballId.value(), true);
	  		}
	  		if (ballMode -> shouldReset){
		  		gameapi -> setGameObjectPosition(ballMode -> ballId.value(), ballMode -> initialBallPos.value(), true, Hint { .hint = "[gamelogic] - ball set pos" });
		  		ballMode -> didLose = false;
					ballMode -> shouldReset = false;
					ballStartTime = gameapi -> timeSeconds(false);
					ballMode -> didReset = true;
	  		}

	  		auto powerup = getPowerup();
	  		if (powerup.has_value()){
		  		std::cout << "powerup: " << print(powerup.value().powerup) << std::endl;
	  			if (powerup.value().powerup == BIG_JUMP){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/jump.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == LAUNCH_FORWARD){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/dash.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == LOW_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == REVERSE_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.normal.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == TELEPORT){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == INVINCIBILITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.normal.png", powerup.value().useTime, powerup.value().duration);
	 		  		return;
	  			}else{
	  				modassert(false, "invalid powerup size ball.cpp");
	  			}
	  		}
	  	}

  		setPowerupTexture("../gameresources/build/textures/ballgame/none.png", std::nullopt, std::nullopt);

  		if (ballMode -> didLose){
	  		gameapi -> drawText("you lose", 0.f, 0.f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}
	  },
	};
	return ballMode;
}

/////////////////////////////////////////////////////////

