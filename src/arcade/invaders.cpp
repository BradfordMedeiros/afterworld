#include "./invaders.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;


// additional features
// momentum based movement
// power ups - automatic, multishot
// shader with lighting? 
// 


struct InvaderParticle {
	glm::vec3 color;
	glm::vec2 position;
	glm::vec2 size;
	glm::vec2 direction;
	float time;
	float duration;
};

struct InvaderEnemy {
	bool markHit;
	glm::vec2 position;
	glm::vec2 size;
};

struct Invaders {
	objid shipId;
	glm::vec2 cursorPosition;
	glm::vec2 shipDirection;

	bool moveLeft;
	bool moveRight;
	bool moveUp;
	bool moveDown;

	bool shoot;
	objid shootingSound;

	std::unordered_map<objid, InvaderParticle> particles;

	float lastEnemySpawnTime;
	std::unordered_map<objid, InvaderEnemy> enemies;

	Collision2DArcade collisions;
};

void createInvadersEnemy(Invaders& invaders, objid id, glm::vec2 position);

glm::vec2 getShipPosition(Invaders& invaders){
	return invaders.enemies.at(invaders.shipId).position;
}

void updateShipPosition(Invaders& invaders, glm::vec2 position){
	invaders.enemies.at(invaders.shipId).position = position;
}

std::any createInvaders(objid id){
	Invaders invaders {
		.shipId = 0,
		.cursorPosition = glm::vec2(0.f, 0.f),
		.shipDirection = glm::vec2(0.f, 0.f),
		.moveLeft = false,
		.moveRight = false,
		.moveUp = false,
		.moveDown = false,
		.shoot = false,
		.particles = {},
		.lastEnemySpawnTime = 0.f,
		.enemies = {},
	};

	arcadeApi.ensureTexturesLoaded(id, 
	{ 
			"../gameresources/textures/arcade/invaders/ship.png", 
			"../gameresources/textures/arcade/invaders/background.png",
	});

	auto soundObjs = arcadeApi.ensureSoundsLoaded(id,
	{
		"./res/sounds/silenced-gunshot2.wav",
		"./res/sounds/glassbreak.wav",
	});
	invaders.shootingSound = soundObjs.at(0);
	invaders.collisions = create2DCollisions();
	createInvadersEnemy(invaders, invaders.shipId, glm::vec2(0.f, 0.f));
	createInvadersEnemy(invaders, getUniqueObjId(), glm::vec2(-0.5f, 0.3f));

	return invaders;
}

void rmInvadersInstance(std::any& any){

}

void createInvadersBullet(Invaders& invaders, glm::vec2 position, glm::vec2 direction){
	auto id = getUniqueObjId();
	invaders.particles[id] = InvaderParticle {
		.color = glm::vec3(1.f, 0.f, 0.f),
		.position = position,
		.size = glm::vec2(0.02f, 0.02f),
		.direction = direction,
		.time = gameapi -> timeSeconds(false),
		.duration = 5.f,
	};
	addColliderRect(invaders.collisions, id, position, glm::vec2(0.02f, 0.02f));
}
void removeInvadersBullet(Invaders& invaders, objid id){
	invaders.particles.erase(id);
	rmCollider(invaders.collisions, id);
}

void createInvadersEnemy(Invaders& invaders, objid id, glm::vec2 position){
	glm::vec2 size(0.1f, 0.1f);
	invaders.enemies[id] = InvaderEnemy {
		.markHit = false,
		.position = position,
		.size = size,
	};
	addColliderRect(invaders.collisions, id, position, size);
}
void removeInvadersEnemy(Invaders& invaders, objid id){
	invaders.enemies.erase(id);
	rmCollider(invaders.collisions, id);
}

void updateInvaderParticles(Invaders& invaders){
	auto elapsedTime = gameapi -> timeElapsed();
	auto currTime = gameapi -> timeSeconds(false);

	std::vector<objid> expiredParticles;
	for (auto &[id, particle] : invaders.particles){
		auto newPosition = particle.position + glm::vec2(elapsedTime * particle.direction.x, elapsedTime * particle.direction.y);
		particle.position = newPosition;
		auto particleExpired = (currTime - particle.time) > particle.duration;
		if (particleExpired){
			expiredParticles.push_back(id);
		}else{
			updatePosition(invaders.collisions, id, particle.position);
		}
	}
	for (auto id : expiredParticles){
		removeInvadersBullet(invaders, id);
	}

	std::vector<objid> killedEnemies;
	for (auto &[id, enemy] : invaders.enemies){
		if (enemy.markHit){
			killedEnemies.push_back(id);
		}
	}
	for (auto id : killedEnemies){
		removeInvadersEnemy(invaders, id);
		arcadeApi.playSound(invaders.shootingSound);
	}
}

// every 5 seconds lets spawn one 
void spawnInvadersEnemies(Invaders& invaders){
	static float minSpawnTime = 2.f;
	auto currentTime = gameapi -> timeSeconds(false);
	if ((currentTime - invaders.lastEnemySpawnTime) < minSpawnTime){
		return;
	}
	if (minSpawnTime > 0.1f){
		minSpawnTime -= 0.05f;
	}

	float degrees = randomNumber(0, 360);
	float radians = glm::radians(degrees);
	auto xValue = glm::cos(radians);
	auto yValue = glm::sin(radians);
	createInvadersEnemy(invaders, getUniqueObjId(), glm::vec2(xValue, yValue));
	invaders.lastEnemySpawnTime = currentTime;
}

void updateInvaders(std::any& any){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

  glm::vec2 direction(0.f, 0.f);
  if (invaders.moveRight){
  	direction.x += 1.f;
  }
  if (invaders.moveLeft){
  	direction.x += -1.f;
  }
  if (invaders.moveUp){
  	direction.y += 1.f;
  }
  if (invaders.moveDown){
  	direction.y += -1.f;
  }

  if (aboutEqual(glm::length(direction), 0.f)){
  	direction = glm::vec2(0.f, 0.f);
  }else{
	  direction = glm::normalize(direction);
  }

  auto timeElapsed = gameapi -> timeElapsed();
  updateShipPosition(invaders, getShipPosition(invaders) + glm::vec2(timeElapsed * direction.x, timeElapsed * direction.y));
  updatePosition(invaders.collisions, invaders.shipId, getShipPosition(invaders));

  spawnInvadersEnemies(invaders);

	if (invaders.shoot){
		createInvadersBullet(invaders, getShipPosition(invaders), invaders.shipDirection);
		arcadeApi.playSound(invaders.shootingSound);
	}
	invaders.shoot = false;
	updateInvaderParticles(invaders);

	// move enemies for now
	for (auto &[id, enemy] : invaders.enemies){
	  enemy.markHit = false; // temp for now

		if (id == invaders.shipId){
			continue;
		}

		auto towardPlayer = glm::normalize(getShipPosition(invaders) - enemy.position);

		float shipSpeed = 0.2f;
		enemy.position += glm::vec2(shipSpeed * timeElapsed * towardPlayer.x, shipSpeed * timeElapsed * towardPlayer.y);
	  updatePosition(invaders.collisions, id, enemy.position);
	}


	auto collisions = getCollisionsArcade(invaders.collisions);
	for (auto &collision : collisions){
		if (collision.obj1 != invaders.shipId){
			if (invaders.enemies.find(collision.obj1) != invaders.enemies.end()){
				invaders.enemies.at(collision.obj1).markHit = true;
			}			
		}
		if (collision.obj2 != invaders.shipId){
			if (invaders.enemies.find(collision.obj2) != invaders.enemies.end()){
				invaders.enemies.at(collision.obj2).markHit = true;
			}
		}
	}
}

void drawInvaders(std::any& any, std::optional<objid> textureId){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

 	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/background.png", std::nullopt);
	gameapi -> drawRect(invaders.cursorPosition.x, invaders.cursorPosition.y, 0.01f, 0.01f, false, glm::vec4(1.f, 0.f, 0.f, 0.5f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	for (auto &[_, enemy] : invaders.enemies){
	 	gameapi -> drawRect(enemy.position.x, enemy.position.y, enemy.size.x, enemy.size.y, false, enemy.markHit ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/ship.png", std::nullopt);
	}
	for (auto &[_, particle] : invaders.particles){
	 	gameapi -> drawRect(particle.position.x, particle.position.y, particle.size.x, particle.size.y, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}
	drawCollisionDebug(invaders.collisions, textureId);
}

void onInvadersMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

	auto direction = glm::vec2(xNdc, yNdc) - getShipPosition(invaders);
	invaders.shipDirection = glm::normalize(direction);
	invaders.cursorPosition = glm::vec2(xNdc, yNdc);
}


void onInvadersMouseClick(std::any& any, int button, int action, int mods){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

	if (button == 0){ // release
		if (action == 1){
			invaders.shoot = true;
		}
	}
}

void onKeyInvaders(std::any& any, int key, int scancode, int action, int mod){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;
	if (key == 'W'){
		if (action == 1){
			invaders.moveUp = true;
		}else if (action == 0){
			invaders.moveUp = false;
		}
	}
	if (key == 'A'){
		if (action == 1){
			invaders.moveLeft = true;
		}else if (action == 0){
			invaders.moveLeft = false;
		}
	}
	if (key == 'S'){
		if (action == 1){
			invaders.moveDown = true;
		}else if (action == 0){
			invaders.moveDown = false;
		}
	}
	if (key == 'D'){
		if (action == 1){
			invaders.moveRight = true;
		}else if (action == 0){
			invaders.moveRight = false;
		}
	}

	if (key == 'R'){
		if (action == 1){
			invaders.shoot = true;
		}
	}
}


ArcadeInterface invadersGame {
	.createInstance = createInvaders,
	.rmInstance = rmInvadersInstance,
	.update = updateInvaders,
	.draw = drawInvaders,
	.onKey = onKeyInvaders,
	.onMouseMove = onInvadersMouseMove,
	.onMouseClick = onInvadersMouseClick,
};

