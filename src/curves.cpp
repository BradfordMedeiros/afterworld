#include "./curves.h"

extern CustomApiBindings* gameapi;

std::unordered_map<objid, std::vector<LinePoints>> rails;  // static-state
std::unordered_map<objid, EntityOnRail> entityToRail; // static-state
std::unordered_map<objid, RaceData> entityToRaceData; // static-state
std::unordered_map<objid, ManagedRailMovement> managedRailMovements;

std::optional<objid> railIdForName(std::string name){
	for (auto& [ownerId, railsForId] : rails){
		for (auto& rail : railsForId){
			if (rail.railName == name){
				return rail.railId;
			}
		}
	}
	return std::nullopt;
}

std::optional<LinePoints*> railForId(objid id){
	for (auto &[ownerId, railsForOwner] : rails){
		for (auto& rail : railsForOwner){
			if (rail.railId == id){
				return &rail;
			}
		}
	}
	return std::nullopt;
}

void drawCurve(LinePoints& line, glm::vec3 point, objid owner){
  for (int i = 0; i < (line.points.size() - 1); i++){
    auto pointFrom = line.points.at(i) + point;
    auto pointTo = line.points.at(i + 1) + point;
    gameapi -> drawLine(pointFrom, pointTo, false, owner, std::nullopt, std::nullopt, std::nullopt);
  }
}

void sortRail(LinePoints& line){
	std::vector<int> arrIndexs;
	for (int i = 0; i < line.points.size(); i++){
		arrIndexs.push_back(i);
	}
	std::sort(arrIndexs.begin(), arrIndexs.end(), [&](int i, int j) {
	    return line.indexs.at(i) < line.indexs.at(j);
	});

	std::vector<glm::vec3> newPoints;
	std::vector<glm::quat> newRotations;
	std::vector<int> newIndexs;
	std::vector<int> newTimes;

  newPoints.reserve(line.points.size());
  newRotations.reserve(line.rotations.size());
  newIndexs.reserve(line.indexs.size());
  newTimes.reserve(line.times.size());

	for (int i = 0; i < arrIndexs.size(); i++){
		newPoints.push_back(line.points.at(arrIndexs.at(i)));
		newRotations.push_back(line.rotations.at(arrIndexs.at(i)));
		newIndexs.push_back(line.indexs.at(arrIndexs.at(i)));	
		newTimes.push_back(line.times.at(arrIndexs.at(i)));
	}
	line.points = newPoints;
	line.rotations = newRotations;
	line.indexs = newIndexs;
	line.times = newTimes;
}

void addRails(objid ownerId, std::vector<RailNode>& railNodes){
	auto railId = getUniqueObjId();

	std::unordered_map<std::string, LinePoints> nameToRail;
	for (auto& node : railNodes){
		auto railName = node.rail;
		nameToRail[railName] = LinePoints {
			.railId = railId,
			.railName = railName,
			.points = {},
			.rotations = {},
			.indexs = {},
			.times = {},
		};
	}

	for (auto& node : railNodes){
		auto& rail = nameToRail.at(node.rail);
		rail.points.push_back(node.point);
		rail.rotations.push_back(node.rotation);
		rail.indexs.push_back(node.railIndex);
		rail.times.push_back(node.time);
	}

	for (auto& [ownerId, railsForId] : rails){
		for (auto& rail : railsForId){
			if (nameToRail.find(rail.railName) != nameToRail.end()){
				modassert(false, "rail name already exists");	
			}
		}
	}

	for (auto&[_, rail] : nameToRail){
		if (rails.find(ownerId) == rails.end()){
			rails[ownerId] = {};
		}
		sortRail(rail);
		rails.at(ownerId).push_back(rail);
	}
}

void removeRails(objid ownerId){
	rails.erase(ownerId);
}

const bool DRAW_CURVES = true;
void drawAllCurves(objid ownerId){
	if (DRAW_CURVES){
  	for (auto &[_, railsForId] : rails){
  		for (auto& line : railsForId){
	  		drawCurve(line, glm::vec3(0.f, 2.f, 0.f), ownerId);
  		}
  	}		
	}
}


struct CurvePoint {
  int lowIndex;
  int highIndex;
  float percentage;
  glm::vec3 point;
};

glm::vec3 calculateNextPointOnRail(LinePoints& linePoints, CurvePoint& curvePoint, float deltaAmount){
	auto lowPoint = linePoints.points.at(curvePoint.lowIndex);
	auto highPoint = linePoints.points.at(curvePoint.highIndex);
	float railLength = glm::distance(highPoint, lowPoint);
	auto alphaAmount = curvePoint.percentage + (deltaAmount / railLength);
	if (alphaAmount > 1.f){
		alphaAmount = 1.f;
	}
	if (alphaAmount < 0.f){
		alphaAmount = 0.f;
	}
	auto value = glm::lerp(lowPoint, highPoint, alphaAmount);
	std::cout << "rail calulateNextPointOnRail, low = " << print(lowPoint) << ", high = " << print(highPoint) << ", target = " << print(value) << ", alpha = " << alphaAmount << std::endl;
	return value;
}


CurvePoint calculateProjectPoint(LinePoints& linePoints, int lowIndex, int highIndex, glm::vec3 point){
	modassert(highIndex < linePoints.points.size(), "calculateProjectPoint highIndex too high");
	auto point1 = linePoints.points.at(lowIndex);
	auto point2 = linePoints.points.at(highIndex);
	//std::cout << "rail lowIndex = " << lowIndex << ", highIndex = " << highIndex << std::endl;

	auto d1 = point2 - point1;
	auto d2 = point - point1;
	auto projectedOnToRailDistance 	= glm::dot(d1, d2);

	float percentage = projectedOnToRailDistance / (glm::length(d1) * glm::length(d2));
	if (percentage != percentage /* check for NaN */ ){
		percentage = 1.f; 
	}
	
	//std::cout << "rail = d1 = " << print(d1) << ", d2 = " << print(d2) << ", percentage = " << percentage << ", projectedOnToRailDistance = " << projectedOnToRailDistance <<  std::endl;
	auto newPoint = point1 + (glm::normalize(d1) * (percentage * glm::length(d2)));

	float percentageToNext = 1.f;
	auto distanceBetween = glm::distance(point1, point2);
	if (distanceBetween != 0.f){
		auto newDistance = glm::distance(newPoint, point1);
		percentageToNext = newDistance / distanceBetween;
		//std::cout << "rail = distanceBetween " <<  distanceBetween <<  ", new distance = " <<  newDistance << ", percentageToNext " << percentageToNext << ", p1 = " << print(newPoint) << ", p2 = " << print(point1) << std::endl;
		if (percentageToNext > 1.f){  // means you went beyond the point, kind of weird
			//std::cout << "rail went beyond" << std::endl;
			percentageToNext = 1.f;
		}		
	}

  return CurvePoint {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentageToNext,
    .point = newPoint,
  };
}


struct LineSegment {
	int lowIndex;
	int highIndex;
};
struct LineSegmentToDistance {
	LineSegment lineSegment;
	float distance;
	glm::vec3 point;
};

std::vector<LineSegmentToDistance> calcSegmentForPoint(LinePoints& linePoints, glm::vec3 point){
	std::vector<LineSegmentToDistance> distances;
	for (int i = 0; i < (linePoints.points.size() - 1); i++){
		auto projectedPoint = calculateProjectPoint(linePoints, i, i+1, point);
		auto distanceToPoint = glm::distance(point, projectedPoint.point);
		distances.push_back(LineSegmentToDistance{
			.lineSegment = LineSegment {
				.lowIndex = i,
				.highIndex = (i + 1),
			},
			.distance = distanceToPoint,
			.point = projectedPoint.point,
		});
	}
	return distances;
}

float getMinDistance(std::vector<LineSegmentToDistance>& segments){
	float minDistance = segments.at(0).distance;
	for (int i = 1; i < segments.size(); i++){
		auto distance = segments.at(i).distance; 
		if (distance < minDistance){
			minDistance = distance;
		}
	}
	return minDistance;
}

bool isCornerPiece(std::vector<int>& minValues, std::vector<LineSegmentToDistance>& segments){
	bool foundFirst = false;
	bool foundEnd = false;

	for (auto minValue : minValues){
		if (segments.at(minValue).lineSegment.highIndex == (segments.size())){
			foundEnd = true;
		}
		if (segments.at(minValue).lineSegment.lowIndex == 0){
			foundFirst = true;
		}
	}
	return foundFirst && foundEnd;
}
LineSegment& pickSegment(std::vector<LineSegmentToDistance>& segments, bool direction){	
	modassert(segments.size() > 0, "no segment to pick");
	std::vector<int> minValues;
	float minDistance = getMinDistance(segments);
	for (int i = 0; i < segments.size(); i++){
		LineSegmentToDistance& segment = segments.at(i);
		if (segment.distance <= (minDistance + 0.1f)){
			minValues.push_back(i);
		}
	}

	auto corner = isCornerPiece(minValues, segments);
	if (corner){
		std::cout << "seg this is a corner" << std::endl;
		if (direction){
			return segments.at(0).lineSegment;
		}else{
			return segments.at(segments.size() - 1).lineSegment;
		}
	}

	if (!direction){
		return segments.at(minValues.at(0)).lineSegment;
	}
	return segments.at(minValues.at(minValues.size() - 1)).lineSegment;
}

CurvePoint calculateCurvePoint(LinePoints& linePoints, glm::vec3 point, bool direction){
	auto closestSegment = calcSegmentForPoint(linePoints, point);
	LineSegment& lineSegment = pickSegment(closestSegment, direction);
	return calculateProjectPoint(linePoints, lineSegment.lowIndex, lineSegment.highIndex, point);
}

float getTotalDistanceForCurve(LinePoints& line){
	float totalDistance = 0.f;
	for (int i = 0; i < (line.points.size() - 1); i++){
		auto point1 = line.points.at(i);
		auto point2 = line.points.at(i + 1);
		auto distance = glm::distance(point1, point2);
		totalDistance += distance;
	}
	return totalDistance;
}
float distanceRemaining(LinePoints& line, int index, float percentageToNext){
	float remainingDistanceAlongCurve = 0.f;
	for (int i = index; i < (line.points.size() - 1); i++){
		auto point1 = line.points.at(i);
		auto point2 = line.points.at(i + 1);
		auto distance = glm::distance(point1, point2);
		if (i == index){
			remainingDistanceAlongCurve += (distance * (1.f - percentageToNext));
		}else{
			remainingDistanceAlongCurve += distance;
		}
	}
	return remainingDistanceAlongCurve;
}

void handleRace(RaceData& raceData, glm::vec3 position, LinePoints& line){
 	auto curvePoint = calculateCurvePoint(line, position, true);
 	raceData.currentPosition = curvePoint.lowIndex;
 	raceData.percentageToNext = curvePoint.percentage;
 	if (raceData.currentPosition > raceData.maxPosition){
 		raceData.maxPosition = raceData.currentPosition;
 	}
 	if (distanceRemaining(line, raceData.currentPosition, raceData.percentageToNext) <= 0.f){
 		raceData.complete = true;
 	}
}

void attachToCurve(objid entityId, objid railId, bool direction){
  entityToRail[entityId] = EntityOnRail {
  	.railId = railId,
  	.direction = direction,
  };
}
void unattachToCurve(objid entityId){
  entityToRail.erase(entityId);
}

void setDirectionCurve(objid entityId, bool direction){
	entityToRail.at(entityId).direction = direction;
}
bool isAttachedToCurve(objid entityId){
	return entityToRail.find(entityId) != entityToRail.end();
}
std::optional<bool> getDirectionCurve(objid entityId){
	if (!isAttachedToCurve(entityId)){
		return std::nullopt;
	}
	return entityToRail.at(entityId).direction;
}
void maybeReverseDirection(objid entityId){
  auto direction = getDirectionCurve(entityId);
  if (direction.has_value()){
    setDirectionCurve(entityId, !direction.value());
  }
}

void addToRace(objid entityId){
	entityToRaceData[entityId] = RaceData {
		.currentPosition = 0,
		.maxPosition = 0,
		.percentageToNext = 0.f,
		.complete = false,
	};
}
void removeFromRace(objid entityId){
	entityToRaceData.erase(entityId);
}

void generateMeshForRail(objid sceneId, LinePoints& linePoints){
	std::vector<glm::vec3> face {
		glm::vec3(-0.05f, 0.f, 0.f),
		glm::vec3(0.f, 0.05f, 0.f),
		glm::vec3(0.05f, 0.f, 0.f),
	};
	std::vector<glm::vec3> points {
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(30.f, 0.f, 0.f),
	};

	std::string meshName = "rail-mesh0";
	gameapi -> generateMesh(face, linePoints.points, meshName);

  GameobjAttributes attr {
    .attr = {
			{ "mesh", meshName },
			{ "position", glm::vec3(0.f, -0.2f, 0.f) },
			{ "texture", "./res/textures/wood.jpg" },
		}
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto id = gameapi -> makeObjectAttr(sceneId, "generatedMesh", attr, submodelAttributes);
}

const float RAIL_GRACE_DISTANCE = 0.5f;
std::optional<NearbyRail> nearbyRail(glm::vec3 position, glm::vec3 forwardDir){
 	for (auto &[ownerId, railsForId] : rails){
 		for (auto& line : railsForId){
			auto points = calcSegmentForPoint(line, position);
			for (int i = 0; i < points.size(); i++){
				auto& point = points.at(i);
				if (point.distance < RAIL_GRACE_DISTANCE){
					auto lowPoint = line.points.at(point.lineSegment.lowIndex);
					auto highPoint = line.points.at(point.lineSegment.highIndex);
					auto railDir = highPoint - lowPoint;
					auto dir = glm::dot(forwardDir, railDir);
					std::cout << "seg: " << dir << std::endl;
					return NearbyRail {
						.id = line.railId,
						.direction = dir >= 0.f,
					};
				}
			}
 		}
 	}
	return std::nullopt;
}

float railLength(LinePoints& line, std::optional<int> index){
	float totalLength = 0;

	auto finalNodeIndex = index.has_value() ? index.value() : (line.points.size() - 1);
	for (int i = 0; i < finalNodeIndex; i++){
		auto fromPoint = line.points.at(i);
		auto toPoint = line.points.at(i + 1);
		totalLength += glm::distance(fromPoint, toPoint);
	}
	return totalLength;
}

struct PointOnRail {
	glm::vec3 position;
	glm::quat rotation;
};
PointOnRail calculatePointOnRail(LinePoints& line, float targetDistance){
	if (line.points.size() == 0){
		std::cout << "calculateProjectPoint: 0 length" << std::endl;
		modassert(false, "no points calculatePointOnRail");
		return PointOnRail {
			.position = glm::vec3(0.f, 0.f, 0.f),
			.rotation = glm::identity<glm::quat>(),
		};
	}
	if (line.points.size() == 1){
		std::cout << "calculateProjectPoint: 1 length" << std::endl;
		return PointOnRail {
			.position = line.points.at(0),
			.rotation = line.rotations.at(0),
		};
	}

	float previousSegmentDistance = 0.f;
	for (int i = 0; i < (line.points.size() - 1); i++){
		auto fromPoint = line.points.at(i);
		auto toPoint = line.points.at(i + 1);

		auto segmentDistance = glm::distance(fromPoint, toPoint);
		if ((previousSegmentDistance + segmentDistance) > targetDistance){
			auto remainingDistance = targetDistance - previousSegmentDistance;
			float fraction = remainingDistance / segmentDistance;
			std::cout << "calculateProjectPoint: returning fractional segment: fromPoint = " << i << ", toPoint = " << (i + 1) << ", fraction = " << fraction << ", targetDistance = " << targetDistance << ", prev = " << previousSegmentDistance << std::endl;

			float x = fromPoint.x + ((toPoint.x - fromPoint.x) * fraction);
			float y = fromPoint.y + ((toPoint.y - fromPoint.y) * fraction);
			float z = fromPoint.z + ((toPoint.z - fromPoint.z) * fraction);
			auto newPoint = glm::vec3(x, y, z);

			auto& lowRotation = line.rotations.at(i);
			auto& highRotation = line.rotations.at(i + 1);
 			
 			glm::quat newRotation = glm::slerp(lowRotation, highRotation, glm::clamp(fraction, 0.0f, 1.0f));

			std::cout << "calculateProjectPoint: point: " << print(newPoint) << std::endl;
			return PointOnRail {
				.position = newPoint,
				.rotation = newRotation,
			};
		}else{
			previousSegmentDistance += segmentDistance;
		}
	}

	auto pos = line.points.at(line.points.size() - 1);
	return PointOnRail {
		.position = pos,
		.rotation = line.rotations.at(line.rotations.size() - 1),
	};
}

struct RailTransform {
	glm::vec3 position;
	glm::quat rotation;
};

RailTransform calculateRelativeOnRail(LinePoints& line, float targetDistance){
	auto rootPoint = line.points.at(0);
	auto pointOnRail = calculatePointOnRail(line, targetDistance);
	auto positionOffset =  pointOnRail.position - rootPoint;
	return RailTransform {
		.position = positionOffset,
		.rotation = pointOnRail.rotation,
	};
}

struct RailSpeed {
	float targetDistance;
	float currentTimeFromStart;
};

int timeToTriggerIndex(LinePoints& line, std::optional<int> index){
	if (!index.has_value()){
		return line.times.at(line.times.size() - 1);
	}
	int totalTime = 0;
	for (int i = 0; i < line.indexs.size(); i++){
		if (i == index.value()){ // shouldnt i be comparing to the value here?
			return line.times.at(i);
		}
	}

	modassert(false, std::string("invalid trigger index: ") + std::to_string(index.value()));
	return 0;
}

RailSpeed calculateRailSpeed(LinePoints& line, ManagedRailMovement& managedRailMovement){
	float elapsedTime = gameapi -> timeSeconds(false) - managedRailMovement.initialStartTime.value();
	auto timeToTrigger = timeToTriggerIndex(line, managedRailMovement.triggerIndex);

	if (managedRailMovement.reverse){
		auto totalTime = timeToTriggerIndex(line, std::nullopt);
		elapsedTime = totalTime - elapsedTime;
	}

	if (managedRailMovement.reverse){
		if (elapsedTime < timeToTrigger){
			elapsedTime = timeToTrigger;
		}
	}else{
		if (elapsedTime > timeToTrigger){
			elapsedTime = timeToTrigger;
		}
	}

	float targetDistance = 0.f;
	for (int i = 0; i < (line.times.size() - 1); i++){
		auto timeOne = line.times.at(i);
		auto timeTwo = line.times.at(i + 1);
		bool isInRange = elapsedTime >= timeOne && elapsedTime < timeTwo;

		float fraction = (elapsedTime - timeOne) / (timeTwo - timeOne);
		if (fraction < 0.f){
			fraction = 0.f;
		}
		if (fraction > 1.f){
			fraction = 1.f;
		}

		std::cout << "time index managedMovement: " << i << ", start = " << managedRailMovement.initialStartTime.value() <<  " , t0 = " << timeOne << ", t1 = " << timeTwo << ", effectiveTime = " << elapsedTime << ", inRange = " << isInRange << ", fraction = " << fraction << std::endl;

		float segmentLength = glm::distance(line.points.at(i), line.points.at(i + 1));
		if (isInRange){
			targetDistance += segmentLength * fraction;
			return RailSpeed {
				.targetDistance = targetDistance,
				.currentTimeFromStart = elapsedTime,
			};
		}
		targetDistance += segmentLength;
	}

	float totalRailLength = railLength(line, std::nullopt);
	return RailSpeed{
		.targetDistance = totalRailLength,
		.currentTimeFromStart = elapsedTime,
	};
}

void triggerMovement(std::string trigger, std::optional<int> railIndex){
	for (auto& [id, managedMovement] : managedRailMovements){
		if (managedMovement.trigger.has_value()){
			if (managedMovement.trigger.value() == trigger){
				std::cout << "triggerMovement: " << (railIndex.has_value() ? railIndex.value() : -1) << std::endl;
				if (!managedMovement.triggerIndex.has_value()){
					managedMovement.initialStartTime = gameapi -> timeSeconds(false);
				}else{
					auto rail = railForId(managedMovement.railId);
					auto newTargetDistance = railLength(*rail.value(), railIndex.value());
					auto railData = calculateRailSpeed(*rail.value(), managedMovement);
					float diff = newTargetDistance - railData.targetDistance;

					if (diff >= 0.f){ 
						managedMovement.reverse = false;
						managedMovement.initialStartTime = gameapi -> timeSeconds(false) - railData.currentTimeFromStart;
					}else {
						managedMovement.reverse = true;
						auto& line = *rail.value();
						float timeFromEnd = line.times.at(line.times.size() - 1);
						managedMovement.initialStartTime = gameapi -> timeSeconds(false) - (timeFromEnd - railData.currentTimeFromStart);
					}
				}
				managedMovement.triggerIndex = railIndex;

			}
		}
	}
}

void handleEntitiesOnRails(objid ownerId, objid sceneId){
	for (auto &[id, managedRailMovement] : managedRailMovements){
		auto rail = railForId(managedRailMovement.railId);

		if (!managedRailMovement.initialStartTime.has_value()){
			if (!managedRailMovement.autostart){
				continue;
			}
			managedRailMovement.initialStartTime = gameapi -> timeSeconds(false);
		}

		auto railSpeed = calculateRailSpeed(*rail.value(), managedRailMovement);
		auto railTransform = calculateRelativeOnRail(*rail.value(), railSpeed.targetDistance);
		auto fullRailTime = timeToTriggerIndex(*rail.value(), std::nullopt);
  	gameapi -> setGameObjectPosition(id, managedRailMovement.initialObjectPos + railTransform.position, true, Hint { .hint = "[gamelogic] - managedRailMovement" });
  	gameapi -> setGameObjectRot(id, railTransform.rotation * managedRailMovement.initialObjectRot, true, Hint { .hint = "[gamelogic] - managedRailMovement rot" });
	
		//std::cout << "rail info: " << railSpeed.endOfRailTime << ", time: " << railSpeed.currentTimeFromStart << std::endl;
		if (managedRailMovement.loop && railSpeed.currentTimeFromStart >= fullRailTime){
			managedRailMovement.initialStartTime = gameapi -> timeSeconds(false);
		}

	}

 	for (auto &[id, entityOnRail] : entityToRail){
 		auto position = gameapi -> getGameObjectPos(id, true, "[gamelogic] curves - entity agent on rail");
  	auto curvePoint = calculateCurvePoint(*railForId(entityOnRail.railId).value(), position, entityOnRail.direction);
  	auto direction = entityOnRail.direction ? 1  : -1;

  	float railDistancePerSecond = 10.f;
  	auto nextPoint = calculateNextPointOnRail(*railForId(entityOnRail.railId).value(), curvePoint, direction * railDistancePerSecond * gameapi -> timeElapsed());
  	gameapi -> setGameObjectPosition(id, nextPoint, true, Hint { .hint = "handleEntitiesOnRails" });
 	}
}

void handleEntitiesRace(){
 	for (auto &[id, raceData] : entityToRaceData){
		static objid activeRaceRailId = 0;
  	auto position = gameapi -> getGameObjectPos(id, true, "[gamelogic] curves - entities race data");
	  handleRace(raceData, position, *railForId(activeRaceRailId).value());
  	auto distance = distanceRemaining(*railForId(activeRaceRailId).value(), raceData.currentPosition, raceData.percentageToNext);
	  auto totalDistance = getTotalDistanceForCurve(*railForId(activeRaceRailId).value());
  	gameapi -> drawText(std::string("race: ") + std::to_string(distance), 0.f, 0.f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	gameapi -> drawText(std::string("percentage: ") + std::to_string(distance / totalDistance), 0.f, -0.1f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	if (raceData.complete){
  	  gameapi -> drawText("race finished!", 0.f, -0.2f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	}
 	}
}