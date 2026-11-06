#pragma once

#include <optional>
#include <vector>
#include "../../../../ModEngine/src/cscript/cscript_binding.h"
#include "../resources/sound.h"

enum NoteState { NOTE_SUCCESS, NOTE_FAILURE, NOTE_DEFAULT };
struct RhythmNoteStat {
  NoteState noteState = NOTE_DEFAULT;
};
struct RhythmNote {
  float time = 0.f;
  int lane = 0;
};
struct RawSongData {
  std::string name;
  std::string displayName;
  std::string file;
  std::vector<RhythmNote> notes;
};

struct RhythmSong {
  float startTime;
  float lastLane0Press = 0.f;
  float lastLane1Press = 0.f;
  float lastLane2Press = 0.f;

  RawSongData* rawSongData = NULL;
  std::vector<RhythmNote> notes;
  std::vector<RhythmNoteStat> stats;
  int songCurrentHealth;
  int songTotalHealth;

  std::optional<objid> clipGameObject;
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
  bool songComplete;
  bool songPlaybackComplete;
  int numNotesTotal;
  int numSuccess;
  int numFailure;
  int songCurrentHealth;
  int songTotalHealth;
};
struct RhythmRendering {
  float currentPlaybackTime;
  std::vector<NoteToRender> notes;
  bool lane0Pressed;
  bool lane1Pressed;
  bool lane2Pressed;

  RawSongData* rawSongData = NULL;
  RhythmStats stats;
};

void loadSong(std::string song);
void unloadSong();

std::optional<RhythmRendering> getRhythmRendering(float length);
void onRhythmFrame();
bool isRhythmEnd();

void onPressKeyRhythm(int key, int action);


std::unordered_map<std::string, RawSongData> loadAllSongs();

void doLoadAllSongs();

void onSongPauseToggle(bool paused);
