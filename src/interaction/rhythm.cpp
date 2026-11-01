#include "./rhythm.h"

extern CustomApiBindings* gameapi;

const float RHYTHM_GRACE = 0.4f;

std::optional<RhythmSong> rhythmSong;
void startRhythm(){
  rhythmSong = RhythmSong {
    .startTime = gameapi -> timeSeconds(false),
  };
}
void rhythmPressNoteLane0(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane0Press = gameapi -> timeSeconds(false);
}
void rhythmPressNoteLane1(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane1Press = gameapi -> timeSeconds(false);
}
void rhythmPressNoteLane2(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane2Press = gameapi -> timeSeconds(false);
}

void registerPressNote(int lane){
  if (lane == 0){
    rhythmPressNoteLane0();
  }
  if (lane == 1){
    rhythmPressNoteLane1();
  }
  if (lane == 2){
    rhythmPressNoteLane2();
  }
}

struct RawSongData {
  std::vector<RhythmNote> notes;
};
std::unordered_map<std::string, RawSongData> songToData {
	{"song1", RawSongData {
		.notes = {
			RhythmNote { 
				.time = 0.f, 
				.lane = 0,
			},
			RhythmNote { 
				.time = 5.f, 
				.lane = 0,
			},
			RhythmNote { 
				.time = 5.f, 
				.lane = 2,
			},
		},
	}},
	{"song2", RawSongData {
		.notes = {
			RhythmNote { },
		},
	}},
};

int initSong1(){
	auto& notes = songToData.at("song1").notes;
	for (int i = 0 ; i < 100; i++){
		notes.push_back(RhythmNote {
			.time = i * 0.2f,
			.lane = i % 3,
		});
	}
	return 0;
}
auto _ = initSong1();

void loadSong(){
	if (rhythmSong.has_value()){
		rhythmSong = std::nullopt;
	}

	rhythmSong = RhythmSong {
		.startTime = gameapi -> timeSeconds(false),
		.lastLane0Press = 0.f,
		.lastLane1Press = 0.f,
		.lastLane2Press = 0.f,
		.notes = songToData.at("song1").notes,
		.stats = {},
		//struct RhythmSong {
		//  float startTime;
		//  float lastLane0Press = 0.f;
		//  float lastLane1Press = 0.f;
		//  float lastLane2Press = 0.f;
		//
		//  std::vector<RhythmNote> notes;
		//  std::vector<RhythmNoteStat> stats;
		//};
	};

	std::vector<RhythmNoteStat> stats;
	for (auto& note : rhythmSong.value().notes){
		stats.push_back(RhythmNoteStat {
			.success = false,
		});
	}
	rhythmSong.value().stats = stats;
}


bool isLane0PressedWithGrace(float currentTime){
	return glm::abs(currentTime - rhythmSong.value().lastLane0Press) <= RHYTHM_GRACE;
}
bool isLane1PressedWithGrace(float currentTime){
	return glm::abs(currentTime - rhythmSong.value().lastLane1Press) <= RHYTHM_GRACE;
}
bool isLane2PressedWithGrace(float currentTime){
	return glm::abs(currentTime - rhythmSong.value().lastLane2Press) <= RHYTHM_GRACE;
}

RhythmRendering getRhythmRendering(float length){
  if (!rhythmSong.has_value()){
    return RhythmRendering {
      .currentPlaybackTime = 0,
      .notes = {},
    };
  }
  auto currentTime = gameapi -> timeSeconds(false);

  std::vector<NoteToRender> notesToRender;
  for (int i = 0; i < rhythmSong.value().notes.size(); i++){
		auto& note = rhythmSong.value().notes.at(i);
		auto& stat = rhythmSong.value().stats.at(i);
  	notesToRender.push_back(NoteToRender {
  		.lane = note.lane,
  		.color = stat.success ? glm::vec4(0.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f),
  		.time = note.time,
  	});
  }
  return RhythmRendering {
    .currentPlaybackTime = currentTime - rhythmSong.value().startTime,
    .notes = notesToRender,
    .lane0Pressed = isLane0PressedWithGrace(currentTime),
    .lane1Pressed = isLane1PressedWithGrace(currentTime),
    .lane2Pressed = isLane2PressedWithGrace(currentTime),
    .stats = RhythmStats {
  		.numNotesTotal = 100,
  		.numSuccess = 34,
  		.numFailure = 5,
    },
  };
}

void onRhythmFrame(){
	if (!rhythmSong.has_value()){
		return;
	}

	auto currentTime = gameapi -> timeSeconds(false);
	auto elapsedTime = currentTime - rhythmSong.value().startTime;

	for (int i = 0; i < rhythmSong.value().notes.size(); i++){
		auto& note = rhythmSong.value().notes.at(i);
		auto& stat = rhythmSong.value().stats.at(i);

		auto adjustedTime = rhythmSong.value().startTime + note.time;
		auto isLane0Pressed = isLane0PressedWithGrace(adjustedTime);
		auto isLane1Pressed = isLane1PressedWithGrace(adjustedTime);
		auto isLane2Pressed = isLane2PressedWithGrace(adjustedTime);

		if (note.lane == 0 && isLane0Pressed){
			stat.success = true;
		}
		if (note.lane == 1 && isLane1Pressed){
			stat.success = true;
		}
		if (note.lane == 2 && isLane2Pressed){
			stat.success = true;
		}
	}
}

void onPressKeyRhythm(int key, int action){
	if (!rhythmSong.has_value()){
		return;
	}
	if (action == 1){
		if (key == 'A'){
    	  registerPressNote(0);
          playMixedSound(getSymbol("ball/rhythm/press_note"), std::nullopt);
    	}
    	if (key == 'S'){
    	  registerPressNote(1);
          playMixedSound(getSymbol("ball/rhythm/press_note"), std::nullopt);
    	}
    	if (key == 'D'){
    	  registerPressNote(2);
          playMixedSound(getSymbol("ball/rhythm/press_note"), std::nullopt);
    	}
	}
}