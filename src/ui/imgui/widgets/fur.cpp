#include "./fur.h"

extern CustomApiBindings* gameapi;

const std::string FUR_SHELL_SHADER_NAME = "../afterworld/shaders/fur_shell/fragment.glsl,../afterworld/shaders/fur_shell/vertex.glsl,../afterworld/shaders/fur_shell/geometry.glsl";

void renderFurShellPanel(bool includePanel) {
  static float furLength = 0.08f;
  static int shellLayers = 6;
  static float strandDensity = 90.0f;
  static float strandWidth = 0.23f;
  static float windMultiplier = 1.0f;

  if (includePanel) {
    ImGui::Begin("Fur Shell");
  }

  ImGui::SliderFloat("Fur Length", &furLength, 0.0f, 0.5f, "%.3f");
  ImGui::SliderInt("Shell Layers", &shellLayers, 2, 11);
  ImGui::SliderFloat("Strand Density", &strandDensity, 20.0f, 180.0f, "%.0f");
  ImGui::SliderFloat("Strand Width", &strandWidth, 0.05f, 0.5f, "%.2f");
  ImGui::SliderFloat("Wind Response", &windMultiplier, 0.0f, 3.0f, "%.2f");

  auto shaderId = gameapi->shaderByName(FUR_SHELL_SHADER_NAME);
  if (shaderId.has_value()) {
    UniformData furLengthUniform {
      .name = "_furLength",
      .value = furLength,
    };
    gameapi->setShaderUniform(shaderId.value(), furLengthUniform);

    UniformData shellLayersUniform {
      .name = "_furLayers",
      .value = shellLayers,
    };
    gameapi->setShaderUniform(shaderId.value(), shellLayersUniform);

    UniformData strandDensityUniform {
      .name = "_furDensity",
      .value = strandDensity,
    };
    gameapi->setShaderUniform(shaderId.value(), strandDensityUniform);

    UniformData strandWidthUniform {
      .name = "_furStrandWidth",
      .value = strandWidth,
    };
    gameapi->setShaderUniform(shaderId.value(), strandWidthUniform);

    UniformData windMultiplierUniform {
      .name = "_furWindMultiplier",
      .value = windMultiplier,
    };
    gameapi->setShaderUniform(shaderId.value(), windMultiplierUniform);
  } else {
    ImGui::TextWrapped("Assign the Fur Shell shader to an object to enable live controls.");
  }

  if (includePanel) {
    ImGui::End();
  }
}
