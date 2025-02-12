#include "./common.h"

extern CustomApiBindings* gameapi;

Collision2DArcade create2DCollisions(){
	return Collision2DArcade{};
}
void addColliderRect(Collision2DArcade& collision, objid id, glm::vec2 pos, glm::vec2 size){
	for (auto object : collision.objs){
		if (id == object.id){
			modassert(false, "addColliderRect duplicate id");
		}
	}
	collision.objs.push_back(Collider2DRect {
		.id = id,
		.position = pos,
		.size = size,
	});
}
void rmCollider(Collision2DArcade& collision, objid id){
	std::vector<Collider2DRect> newObjs;
	for (auto &obj : collision.objs){
		if (obj.id == id){
			continue;
		}
		newObjs.push_back(obj);
	}
	collision.objs = newObjs;
}
void updatePosition(Collision2DArcade& collision, objid id, glm::vec2 pos){
	for (auto &obj : collision.objs){
		if (obj.id == id){
			obj.position = pos;
			return;
		}
	}
}

bool checkOverlapCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2){
	auto firstLeft = pos1.x - (0.5f * size1.x);
	auto firstRight = pos1.x + (0.5f * size1.x);
	auto firstUp = pos1.y + (0.5f * size1.y);
	auto firstDown = pos1.y - (0.5f * size1.y);

	auto secLeft = pos2.x - (0.5f * size2.x);
	auto secRight = pos2.x + (0.5f * size2.x);
	auto secUp = pos2.y + (0.5f * size2.y);
	auto secDown = pos2.y - (0.5f * size2.y);

	bool overlapsX = (firstLeft >= secLeft && firstLeft <= secRight) || (firstRight >= secLeft && firstRight <= secRight);
	bool overlapsY = (firstUp >= secDown && firstUp <= secUp) || (firstDown >= secDown && firstDown <= secUp);
	return overlapsX && overlapsY;
}

// This can obviously be made way more efficient, just n^2 style comparison for now
std::vector<Collision2D> getCollisionsArcade(Collision2DArcade& collision){
	std::vector<Collision2D> ids;
	for (int i = 0; i < collision.objs.size(); i++){
		for (int j = (i + 1); j < collision.objs.size(); j++){
			Collider2DRect& obj1 = collision.objs.at(i);
			Collider2DRect& obj2 = collision.objs.at(j);
			auto overlaps1 = checkOverlapCollision(obj1.position, obj1.size, obj2.position, obj2.size);
			auto overlaps2 = checkOverlapCollision(obj2.position, obj2.size, obj1.position, obj1.size);
			auto overlaps = overlaps1 || overlaps2;
			if (overlaps){
				ids.push_back(Collision2D {
					.obj1 = obj1.id,
					.obj2 = obj2.id,
				});				
			}
		}	
	}
	return ids;
}

void drawCollisionDebug(Collision2DArcade& collision, std::optional<objid> textureId){
	for (auto &obj : collision.objs){
		float left = obj.position.x - (0.5f * obj.size.x);
		float right = obj.position.x + (0.5f * obj.size.x);
		float top = obj.position.y + (0.5f * obj.size.y);
		float bottom = obj.position.y - (0.5f * obj.size.y);

		glm::vec3 topLeft(left, top, 0.f);
		glm::vec3 topRight(right, top, 0.f);
		glm::vec3 bottomLeft(left, bottom, 0.f);
		glm::vec3 bottomRight(right, bottom, 0.f);

		gameapi -> drawLine2D(topLeft, topRight, false, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
		gameapi -> drawLine2D(bottomLeft, bottomRight, false, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
		gameapi -> drawLine2D(topLeft, bottomLeft, false, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
		gameapi -> drawLine2D(topRight, bottomRight, false, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}
}

glm::vec2 rotatePoint(glm::vec2 point, glm::vec2 dir){
	auto angleRadians = atanRadians360(dir.y, dir.x);
	glm::mat2 rotationMatrix = glm::mat2(glm::cos(angleRadians), -glm::sin(angleRadians), glm::sin(angleRadians), glm::cos(angleRadians));
	glm::vec2 rotatedPoint = rotationMatrix * point;
	return rotatedPoint;
}

void drawCenteredTextFade(const char* text, float x, float y, float size, glm::vec3 color, float period, std::optional<objid> textureId){
	float alphaValue = gameapi -> timeSeconds(false) / period;
  float percentage = fmod(alphaValue, 1);
  if (percentage < 0.5f){
  	percentage *= 2;
  }else{
  	percentage = 2.f - (2.f * percentage);
  }
  drawCenteredText(text, x, y, size, glm::vec4(color.x, color.y, color.z, percentage), std::nullopt, textureId);
}
