#pragma once

#include <optional>
#include <vector>
#include "../../../../ModEngine/src/cscript/cscript_binding.h"
#include "../resources/sound.h"

struct RhythmNoteStat {
  bool success;
};
struct RhythmNote {
  float time = 0.f;
  int lane = 0;
};

struct RhythmSong {
  float startTime;
  float lastLane0Press = 0.f;
  float lastLane1Press = 0.f;
  float lastLane2Press = 0.f;

  std::vector<RhythmNote> notes;
  std::vector<RhythmNoteStat> stats;

  std::optional<OneShot> playingSong;
};

void startRhythm();
void rhythmPressNoteLane0();
void rhythmPressNoteLane1();
void rhythmPressNoteLane2();
void registerPressNote(int lane);

struct NoteToRender {
  int lane;
  glm::vec4 color;
  float time;
};

struct RhythmStats {
  int numNotesTotal;
  int numSuccess;
  int numFailure;
};
struct RhythmRendering {
  float currentPlaybackTime;
  std::vector<NoteToRender> notes;
  bool lane0Pressed;
  bool lane1Pressed;
  bool lane2Pressed;

  RhythmStats stats;
};

void loadSong(std::string song);
void unloadSong();

RhythmRendering getRhythmRendering(float length);
void onRhythmFrame();

void onPressKeyRhythm(int key, int action);