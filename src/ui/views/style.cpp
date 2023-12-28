#include "./style.h"

extern CustomApiBindings* gameapi;

const bool DEBUG_STYLES = false;
void initStyles(){
  auto query = gameapi -> compileSqlQuery("select style-primary, style-secondary, style-border, style-highlight from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  auto primaryColor = parseVec4(result.at(0).at(0));
  auto secondaryColor = parseVec4(result.at(0).at(1));
  auto borderColor = parseVec4(result.at(0).at(2));
  auto highlightColor = parseVec4(result.at(0).at(3));

  styles.primaryColor = primaryColor;
  styles.secondaryColor = secondaryColor;
  styles.mainBorderColor = borderColor;
  styles.highlightColor = highlightColor;
}

Styles style1 {
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.5f),
	.secondaryColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f),
	.thirdColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.5f),
	.highlightColor = glm::vec4(0.f, 1.f, 0.f, 0.5f),
	.debugColor = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.debugColor2 = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
	.dockElementPadding = 0.02f,
};

Styles styles = style1;


