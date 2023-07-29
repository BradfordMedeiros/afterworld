#include "./common.h"

extern CustomApiBindings* gameapi;

void drawDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint){
  //gameapi -> drawRect(box.x, box.y, box.width, box.height, false, glm::vec4(1.f, 0.f, 1.f, 0.2f), std::nullopt, true, std::nullopt, std::nullopt);
  //gameapi -> drawRect(-0.915000, -0.795, 0.17f, 0.15f, false, glm::vec4(1.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  float left = box.x - (box.width * 0.5f);
  float right = box.x + (box.width * 0.5f);
  float up = box.y + (box.height * 0.5f);
  float down = box.y - (box.height * 0.5f);
  drawTools.drawLine2D(glm::vec3(left, up, 0.f), glm::vec3(right, up, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(left, down, 0.f), glm::vec3(right, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(left, up, 0.f), glm::vec3(left, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(right, up, 0.f), glm::vec3(right, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
}

std::string print(BoundingBox2D& box){
  return std::string("x = " + std::to_string(box.x) + ", y = " + std::to_string(box.y) + ", width = " + std::to_string(box.width) + ", height = " + std::to_string(box.height));
}

void drawScreenspaceGrid(ImGrid grid){
  float numLines = grid.numCells - 1;
  float ndiSpacePerLine = 1.f / grid.numCells;

  for (int y = 0; y < numLines; y ++){
    float unitLineNdi = ndiSpacePerLine * (y + 1);
    float ndiY = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(-1.f, ndiY, 0.f), glm::vec3(1.f, ndiY, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
  for (int x = 0; x < numLines; x ++){
    float unitLineNdi = ndiSpacePerLine * (x + 1);
    float ndiX = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(ndiX, -1.f, 0.f), glm::vec3(ndiX, 1.f, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
}

