#include "./sound.h"

std::vector<std::string> listSoundFiles();


struct SoundFolderNode{
    std::string name;
    std::vector<int> sounds;
    std::vector<SoundFolderNode> children;
};
SoundFolderNode BuildSoundTree(std::vector<SoundBinding>& soundBindings){
    SoundFolderNode root;
    for (int i = 0; i < soundBindings.size(); i++){
        auto& binding = soundBindings.at(i);
        SoundFolderNode* node = &root;
        for (auto& folder : binding.folder){
            auto it = std::find_if(
                node->children.begin(),
                node->children.end(),
                [&](const SoundFolderNode& child)
                {
                    return child.name == folder;
                });

            if (it == node->children.end()){
                node->children.push_back(SoundFolderNode{
                    .name = folder
                });
                node = &node->children.back();
            }else{
                node = &*it;
            }
        }
        node->sounds.push_back(i);
    }
    return root;
}

void DrawSoundTree(
    const SoundFolderNode& node,
    const std::vector<SoundBinding>& soundBindings,
    int& selectedSound){
    for (const auto& child : node.children){
        if (ImGui::TreeNode(child.name.c_str())){
            DrawSoundTree(child, soundBindings, selectedSound);
            ImGui::TreePop();
        }
    }

    for (int index : node.sounds){
        const auto& sound = soundBindings[index];
        if (ImGui::Selectable(sound.sound.c_str(), selectedSound == index)){
            selectedSound = index;
        }
    }
}


void renderMixingPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Mixing");
  }

  auto soundBindings = getSoundInfo().soundBindings;
  SoundFolderNode soundTree = BuildSoundTree(soundBindings);

  int selectedSound = -1;
  DrawSoundTree(soundTree, soundBindings, selectedSound);
  if (selectedSound != -1){
    std::cout << "selected sound: " << selectedSound <<  " " << soundBindings.at(selectedSound).sound << std::endl;
    setActiveMixedSound(soundBindings.at(selectedSound));
  }

  if (includePanel){
    ImGui::End();
  }    
}

void renderMixingDetailPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Mixing Detail");
  }

  auto mixedSoundName = activeMixedSound();
  if (!mixedSoundName.has_value()){
    if (includePanel){
      ImGui::End();
    }
    return;    
  }


  auto mixedSoundPtr = getMixedSound(mixedSoundName.value());
  auto& mixedSound = *mixedSoundPtr.value();


  ImGui::Text("Sound Name:");
  ImGui::SameLine();
  ImGui::Text(mixedSound.soundBinding.sound.c_str());

  if (ImGui::Button("Play")){
    playMixedSound(mixedSound.nameSymbol, std::nullopt);
  }
  ImGui::SameLine();
  if (ImGui::Button("Stop")){

  }
  ImGui::SameLine();
  if (ImGui::Button("Save")){
    saveMixedSound(mixedSound);
  }

  ImGui::Checkbox("Loop", &mixedSound.loop);
  ImGui::Checkbox("Center", &mixedSound.center);
  ImGui::SliderFloat("Volume", &mixedSound.volume, 0.f, 1.f);

  std::vector<std::string> clips = listSoundFiles();

  for (int i = 0; i < 5; i++){
    std::optional<std::string> currClip = mixedSound.clips.size() > i ? mixedSound.clips.at(i) : std::optional<std::string>(std::nullopt);
  
    bool enableSound = currClip.has_value();
    bool wasEnableSound = enableSound;

    std::string value = (std::string("Enable Sound ") + std::to_string(i));
    ImGui::Checkbox(value.c_str(), &enableSound);
    if (enableSound && !currClip.has_value()){
      currClip = clips.at(0);
      enableMixedSoundClip(mixedSound, i);
      std::cout << "enable curr clip" << std::endl;
    }
    if (!enableSound && wasEnableSound){
      disableMixedSoundClip(mixedSound, i);
      currClip = std::nullopt;
    }

    std::cout << "enable curr name: " << print(currClip) << std::endl;
    if (currClip.has_value()){
      std::cout << "enable curr drawCombo" << std::endl;

      if (ImGui::BeginCombo((std::string("Clip: ") + std::to_string(i)).c_str(), currClip.value().c_str())){
        for (int j = 0; j < clips.size(); j++){
          bool selected =  currClip.value() == clips.at(j);
          if (ImGui::Selectable(clips.at(j).c_str(), selected)){
            if (i == 0){
              setMixedSoundClip(mixedSound, clips.at(j), 0);
            }else if (i == 1){
              setMixedSoundClip(mixedSound, clips.at(j), 1);
            }else if (i == 2){
              setMixedSoundClip(mixedSound, clips.at(j), 2);
            }else if (i == 3){
              setMixedSoundClip(mixedSound, clips.at(j), 3);
            }else if (i == 4){
              setMixedSoundClip(mixedSound, clips.at(j), 4);
            }
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
    }    
  }


  bool isSequential = mixedSound.clipOrderSequential;
  bool wasSequential = isSequential;
  bool isRandom = !mixedSound.clipOrderSequential;
  bool wasRandom = isRandom;

  ImGui::Checkbox("Sequential", &isSequential);
  ImGui::SameLine();
  ImGui::Checkbox("Random", &isRandom);
  if (isSequential && !wasSequential){
    mixedSound.clipOrderSequential = true;
  }else if (isRandom && !wasRandom){
    mixedSound.clipOrderSequential = false;
  }


  std::vector<std::string> buses = busNames();

  if (ImGui::BeginCombo("Bus", soundBusToStr(mixedSound.bus).c_str())){
    for (int i = 0; i < buses.size(); i++){
      bool selected = soundBusToStr(mixedSound.bus) == buses.at(i);
      if (ImGui::Selectable(buses.at(i).c_str(), selected)){
        auto selectedBusStr = buses.at(i);
        auto soundBus = stringToSoundBus(selectedBusStr);
        mixedSound.bus = soundBus;
      }
      if (selected){
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  if (includePanel){
    ImGui::End();
  }    
}
