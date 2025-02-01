#include "./invaders.h"

#include "./tennis.h"

extern CustomApiBindings* gameapi;

struct InvaderCollisionObj {
	glm::vec2 position;
};

struct InvaderParticle {
	glm::vec3 color;
	glm::vec2 position;
	glm::vec2 direction;
	float time;
	float duration;
};

struct InvaderEnemy {
	glm::vec2 position;
};

struct Invaders {
	glm::vec2 cursorPosition;
	glm::vec2 shipPosition;
	glm::vec2 shipDirection;

	bool moveLeft;
	bool moveRight;
	bool moveUp;
	bool moveDown;

	bool shoot;
	std::unordered_map<objid, InvaderParticle> particles;
	std::unordered_map<objid, InvaderEnemy> enemies;
	std::unordered_map<objid, InvaderCollisionObj> collisionObjs;
};

void createInvadersEnemy(Invaders& invaders, glm::vec2 position);


std::any createInvaders(){
	Invaders invaders {
		.cursorPosition = glm::vec2(0.f, 0.f),
		.shipPosition = glm::vec2(0.f, 0.f),
		.shipDirection = glm::vec2(0.f, 0.f),
		.moveLeft = false,
		.moveRight = false,
		.moveUp = false,
		.moveDown = false,
		.shoot = false,
		.particles = {},
		.enemies = {},
		.collisionObjs = {},
	};

	createInvadersEnemy(invaders, glm::vec2(-0.5f, 0.5f));

	return invaders;
}

void rmInvadersInstance(std::any& any){

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


void createInvadersBullet(Invaders& invaders, glm::vec2 position, glm::vec2 direction){
	auto id = getUniqueObjId();
	invaders.particles[id] = InvaderParticle {
		.color = glm::vec3(1.f, 0.f, 0.f),
		.position = position,
		.direction = direction,
		.time = gameapi -> timeSeconds(false),
		.duration = 5.f,
	};
}

void createInvadersEnemy(Invaders& invaders, glm::vec2 position){
	auto id = getUniqueObjId();
	invaders.enemies[id] = InvaderEnemy {
		.position = position,
	};
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
		}
	}

	for (auto id : expiredParticles){
		invaders.particles.erase(id);
	}
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
  invaders.shipPosition += glm::vec2(timeElapsed * direction.x, timeElapsed * direction.y);

	if (invaders.shoot){
		createInvadersBullet(invaders, invaders.shipPosition, invaders.shipDirection);
	}
	invaders.shoot = false;
	updateInvaderParticles(invaders);
}

void drawInvaders(std::any& any, std::optional<objid> textureId){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

 	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, "./res/textures/testgradient.png", std::nullopt);
	gameapi -> drawRect(invaders.shipPosition.x, invaders.shipPosition.y, 0.1f, 0.1f, false, glm::vec4(1.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

  //gameapi -> drawLine2D(
  //	glm::vec3(invaders.shipPosition.x, invaders.shipPosition.y, 0.f), 
  //	glm::vec3(invaders.shipPosition.x + invaders.shipDirection.x, invaders.shipPosition.y + invaders.shipDirection.y, 0.f), 
  //	false, 
  //	glm::vec4(1.f, 0.f, 0.f, 1.f), 
  //	textureId, 
  //	true,
  //	std::nullopt, 
  //	std::nullopt /* texture id */, 
  //	std::nullopt);

	gameapi -> drawRect(invaders.cursorPosition.x, invaders.cursorPosition.y, 0.01f, 0.01f, false, glm::vec4(1.f, 0.f, 0.f, 0.5f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	for (auto &[_, particle] : invaders.particles){
	 	gameapi -> drawRect(particle.position.x, particle.position.y, 0.02f, 0.02f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}

	for (auto &[_, enemy] : invaders.enemies){
	 	gameapi -> drawRect(enemy.position.x, enemy.position.y, 0.1f, 0.1f, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}

}

void onInvadersMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
  Invaders* invadersPtr = anycast<Invaders>(any);
  Invaders& invaders = *invadersPtr;

	auto direction = glm::vec2(xNdc, yNdc) - invaders.shipPosition;

	invaders.shipDirection = direction;
	invaders.cursorPosition = glm::vec2(xNdc, yNdc);
}


ArcadeInterface invadersGame {
	.createInstance = createInvaders,
	.rmInstance = rmInvadersInstance,
	.update = updateInvaders,
	.draw = drawInvaders,
	.onKey = onKeyInvaders,
	.onMouseMove = onInvadersMouseMove,
};

