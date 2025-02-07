#include "./invaders.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;

// power ups - automatic, multishot
// shader with lighting? 

struct InvaderParticle {
	glm::vec4 color;
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
	glm::vec4 color;
};

enum InvadersState { TITLE, PLAYING };
struct Invaders {
	InvadersState state;
	int score;
	float minSpawnTime;

	bool bulletColorAlt;
	objid shipId;
	glm::vec2 cursorPosition;
	glm::vec2 shipDirection;
	glm::vec2 velocity;


	std::vector<glm::vec2> shipTrail;

	bool moveLeft;
	bool moveRight;
	bool moveUp;
	bool moveDown;

	bool shoot;
	objid shootingSound;
	unsigned int* shaderId;

	std::unordered_map<objid, InvaderParticle> particles;

	float lastEnemySpawnTime;
	std::unordered_map<objid, InvaderEnemy> enemies;

	Collision2DArcade collisions;
};

void createInvadersEnemy(Invaders& invaders, objid id, glm::vec2 position, glm::vec4 color);

glm::vec2 getShipPosition(Invaders& invaders){
	return invaders.enemies.at(invaders.shipId).position;
}

void updateShipPosition(Invaders& invaders, glm::vec2 position){
	invaders.enemies.at(invaders.shipId).position = position;
}

Invaders initialInvadersState {
	.state = TITLE,
	.score = 0,
	.minSpawnTime = 2.f,
	.bulletColorAlt = false,
	.shipId = 0,
	.cursorPosition = glm::vec2(0.f, 0.f),
	.shipDirection = glm::vec2(0.f, 0.f),
	.velocity = glm::vec2(0.f, 0.f),
	.shipTrail = {},
	.moveLeft = false,
	.moveRight = false,
	.moveUp = false,
	.moveDown = false,
	.shoot = false,
	.particles = {},
	.lastEnemySpawnTime = 0.f,
	.enemies = {},
};
std::any createInvaders(objid id){
	Invaders invaders = initialInvadersState;

	invaders.shaderId = gameapi -> loadShader("arcade", "../afterworld/shaders/arcade");


	arcadeApi.ensureTexturesLoaded(id, 
	{ 
			"../gameresources/textures/arcade/invaders/ship.png", 
			"../gameresources/textures/arcade/invaders/background.png",
			"../gameresources/textures/arcade/invaders/pete.png", 
	});

	auto soundObjs = arcadeApi.ensureSoundsLoaded(id,
	{
		"./res/sounds/silenced-gunshot2.wav",
		"./res/sounds/glassbreak.wav",
	});
	invaders.shootingSound = soundObjs.at(0);
	invaders.collisions = create2DCollisions();
	createInvadersEnemy(invaders, invaders.shipId, glm::vec2(0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f));

	return invaders;
}

void clearInvadersState(Invaders& invaders){
	invaders.state = TITLE;
	invaders.score = 0;
	invaders.minSpawnTime = 2.f;
	invaders.bulletColorAlt = false;

	invaders.collisions = create2DCollisions();
	invaders.cursorPosition = glm::vec2(0.f, 0.f); 
	invaders.shipDirection = glm::vec2(0.f, 0.f);
	invaders.velocity = glm::vec2(0.f, 0.f);
	invaders.shipTrail = {};
	invaders.moveLeft = false;
	invaders.moveRight = false;
	invaders.moveDown = false;
	invaders.moveUp = false;
	invaders.shoot = false;
	invaders.particles = {};
	invaders.lastEnemySpawnTime = 0.f;
	invaders.enemies = {};
	createInvadersEnemy(invaders, invaders.shipId, glm::vec2(0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f));
}

void rmInvadersInstance(std::any& any){
}

void createInvadersBullet(Invaders& invaders, glm::vec2 position, glm::vec2 direction){
	auto id = getUniqueObjId();
	invaders.particles[id] = InvaderParticle {
		.color = invaders.bulletColorAlt ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f),
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

void createInvadersEnemy(Invaders& invaders, objid id, glm::vec2 position, glm::vec4 color){
	glm::vec2 size(0.1f, 0.1f);
	invaders.enemies[id] = InvaderEnemy {
		.markHit = false,
		.position = position,
		.size = size,
		.color = color,
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
		invaders.score = invaders.score + 100;
	}
}

// every 5 seconds lets spawn one 
void spawnInvadersEnemies(Invaders& invaders){
	if (invaders.enemies.size() > 20){
		return;
	}
	auto currentTime = gameapi -> timeSeconds(false);
	if ((currentTime - invaders.lastEnemySpawnTime) < invaders.minSpawnTime){
		return;
	}
	if (invaders.minSpawnTime > 0.5f){
		invaders.minSpawnTime -= 0.05f;
	}

	float degrees = randomNumber(0, 360);
	float radians = glm::radians(degrees);
	auto xValue = 1.5 * glm::cos(radians);
	auto yValue = 1.5 * glm::sin(radians);

	auto teamNumber = randomNumber(0, 10);

	createInvadersEnemy(invaders, getUniqueObjId(), glm::vec2(xValue, yValue), teamNumber > 3 ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f));
	invaders.lastEnemySpawnTime = currentTime;
}

bool objIsEnemy(Invaders& invaders, objid id){
	return invaders.enemies.find(id) != invaders.enemies.end();
}
bool objIsParticle(Invaders& invaders, objid id){
	return invaders.particles.find(id) != invaders.particles.end();
}
bool objIsShip(Invaders& invaders, objid id){
	return invaders.shipId == id;
}

void updateShipTrail(Invaders& invaders){
	auto shipPos = getShipPosition(invaders);
	if (invaders.shipTrail.size() == 0){
		invaders.shipTrail.push_back(shipPos);
		return;
	}

	if (invaders.shipTrail.size() > 10){
		std::vector<glm::vec2> newValues;
		for (int i = 1; i < invaders.shipTrail.size(); i++){
			newValues.push_back(invaders.shipTrail.at(i));
		}
		invaders.shipTrail = newValues;
	}

	auto lastTrailPosition = invaders.shipTrail.at(invaders.shipTrail.size() - 1);
	if (glm::distance(shipPos, lastTrailPosition) > 0.02f){
		invaders.shipTrail.push_back(shipPos);
	}


}

void updateInvaders(std::any& any){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

  if (invaders.state == TITLE){
  	return;
  }

  float multiplier = (1.f - (5 * gameapi -> timeElapsed() * 0.99f));
  if (multiplier > 0){
	  invaders.velocity.x *= multiplier;
	  invaders.velocity.y *= multiplier;
  }


  if (invaders.moveRight){
  	invaders.velocity.x += 0.02f;
  }
  if (invaders.moveLeft){
  	invaders.velocity.x += -0.02f;
  }
  if (invaders.moveUp){
  	invaders.velocity.y += 0.02f;
  }
  if (invaders.moveDown){
  	invaders.velocity.y += -0.02f;
  }

  // might want the overall speed not components 
  if (invaders.velocity.x > 2){
  	invaders.velocity.x = 2.f;
  }
  if (invaders.velocity.y > 2){
  	invaders.velocity.y = 2.f; 
  }

  auto direction = invaders.velocity;

  auto timeElapsed = gameapi -> timeElapsed();
  updateShipPosition(invaders, getShipPosition(invaders) + glm::vec2(timeElapsed * direction.x, timeElapsed * direction.y));
  updatePosition(invaders.collisions, invaders.shipId, getShipPosition(invaders));

  updateShipTrail(invaders);

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

	bool gameFinished = false;

	auto collisions = getCollisionsArcade(invaders.collisions);
	for (auto &collision : collisions){
		bool obj1IsEnemy = objIsEnemy(invaders, collision.obj1);
		bool obj2IsEnemy = objIsEnemy(invaders, collision.obj2);
		bool obj1IsShip = objIsShip(invaders, collision.obj1);
		bool obj2IsShip = objIsShip(invaders, collision.obj2);
		bool obj1IsBullet = objIsParticle(invaders, collision.obj1);
		bool obj2IsBullet = objIsParticle(invaders, collision.obj2);

		if (obj1IsBullet && (obj2IsEnemy && !obj2IsShip)){
			auto colorEnemy = invaders.enemies.at(collision.obj2).color;
			auto colorBullet = invaders.particles.at(collision.obj1).color;
			if (aboutEqual(colorEnemy, colorBullet)){
				invaders.enemies.at(collision.obj2).markHit = true;
				removeInvadersBullet(invaders, collision.obj1);				
			}
		}else if (obj2IsBullet && (obj1IsEnemy && !obj1IsShip)){
			auto colorEnemy = invaders.enemies.at(collision.obj1).color;
			auto colorBullet = invaders.particles.at(collision.obj2).color;
			if (aboutEqual(colorEnemy, colorBullet)){
				invaders.enemies.at(collision.obj1).markHit = true;
				removeInvadersBullet(invaders, collision.obj2);				
			}
		}

		if (obj1IsEnemy && obj2IsShip){
			gameFinished = true;
		}else if (obj2IsEnemy && obj1IsShip){
			gameFinished = true;
		}
	}
	if (gameFinished){
		clearInvadersState(invaders);
	}
}

void drawCenteredTextFade(const char* text, float x, float y, float size, glm::vec3 color, float period){
	float alphaValue = gameapi -> timeSeconds(false) / period;
  float percentage = fmod(alphaValue, 1);
  if (percentage < 0.5f){
  	percentage *= 2;
  }else{
  	percentage = 2.f - (2.f * percentage);
  }
  drawCenteredText(text, x, y, size, glm::vec4(color.x, color.y, color.z, percentage), std::nullopt);
}

void drawInvaders(std::any& any, std::optional<objid> textureId){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

  if (invaders.state == TITLE){
   	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/background.png", ShapeOptions { .shaderId = *invaders.shaderId  });
   	gameapi -> drawRect(0.f, 0.f + 0.02f * glm::cos(gameapi -> timeSeconds(false) * 2.f), 0.4f, 0.4f, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/pete.png", std::nullopt);
	  drawCenteredTextFade("Press Left Mouse to Play!", 0, -0.4f, 0.04f, glm::vec3(1.f, 1.f, 1.f), 2.f);
  	return;
  }

 	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/background.png", ShapeOptions { .shaderId = *invaders.shaderId  });
	gameapi -> drawRect(invaders.cursorPosition.x, invaders.cursorPosition.y, 0.01f, 0.01f, false, glm::vec4(1.f, 0.f, 0.f, 0.5f), textureId, true, std::nullopt, std::nullopt, std::nullopt);


	for (int i = 0; i < invaders.shipTrail.size(); i++){
		auto trailPos = invaders.shipTrail.at(i);
		float opacity = static_cast<float>(i) / static_cast<float>(invaders.shipTrail.size());
	 	gameapi -> drawRect(trailPos.x, trailPos.y, 0.1f, 0.1f, false, glm::vec4(0.f, 0.f, 1.f, 0.2f * opacity), textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/ship.png", ShapeOptions { .shaderId = *invaders.shaderId  });
	}
	for (auto &[_, enemy] : invaders.enemies){
	 	gameapi -> drawRect(enemy.position.x, enemy.position.y, enemy.size.x, enemy.size.y, false, enemy.color, textureId, true, std::nullopt, "../gameresources/textures/arcade/invaders/ship.png", ShapeOptions { .shaderId = *invaders.shaderId  });
	}

	for (auto &[_, particle] : invaders.particles){
	 	gameapi -> drawRect(particle.position.x, particle.position.y, particle.size.x, particle.size.y, false, particle.color, textureId, true, std::nullopt, std::nullopt, ShapeOptions { .shaderId = *invaders.shaderId  });
	}
	//drawCollisionDebug(invaders.collisions, textureId);
  drawRightText(std::string("SCORE: ")  + std::to_string(invaders.score), -0.975f, 0.95, 0.05, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt);
  gameapi -> drawRect(0.f, 0.9f, 0.9f, 0.1f, false, glm::vec4(0.f, 0.f, 0.f, 0.8f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
  gameapi -> drawRect(0.f, 0.9f, (invaders.score / 10000.f) * 0.9f, 0.1f, false, glm::vec4(0.f, 0.f, 1.f, 0.8f), textureId, true, std::nullopt, std::nullopt, std::nullopt);


  //gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.f, 1.f, 0.f, 0. * glm::cos(gameapi -> timeSeconds(false) * 0.1f)), textureId, true, std::nullopt, std::nullopt, std::nullopt);

}

void onInvadersMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

  if (invaders.state == TITLE){
  	return;
  }

	auto direction = glm::vec2(xNdc, yNdc) - getShipPosition(invaders);
	invaders.shipDirection = glm::normalize(direction);
	invaders.cursorPosition = glm::vec2(xNdc, yNdc);
}


void onInvadersMouseClick(std::any& any, int button, int action, int mods){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;


  if (invaders.state == TITLE){
  	if (button == 0 && action == 0){
  		invaders.state = PLAYING;
			arcadeApi.playSound(invaders.shootingSound);
  	}
  	return;
  }

	if (button == 0){ // release
		if (action == 1){
			invaders.shoot = true;
		}
	}
	if (button == 1){
		if (action == 1){
			invaders.bulletColorAlt = !invaders.bulletColorAlt;
		}
	}
}

void onKeyInvaders(std::any& any, int key, int scancode, int action, int mod){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;


  if (invaders.state == TITLE){
  	return;
  }

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

