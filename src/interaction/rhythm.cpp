#include "./rhythm.h"
#include "../resources/paths.h"

extern CustomApiBindings* gameapi;

const float RHYTHM_GRACE = 0.1f;


int getDefaultPlayerIndex();
void applyUiScreenshakeByPlayerIndex(int playerIndex, glm::vec2 impulse);

std::optional<RhythmSong> rhythmSong;

void rhythmPressNoteLane0(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane0Press = gameapi -> timeSeconds(false);
	rhythmSong.value().lane0PressLastFrame = true;
}
void rhythmPressNoteLane1(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane1Press = gameapi -> timeSeconds(false);
	rhythmSong.value().lane1PressLastFrame = true;
}
void rhythmPressNoteLane2(){
  if (!rhythmSong.has_value()){
    return;
  }
  rhythmSong.value().lastLane2Press = gameapi -> timeSeconds(false);
	rhythmSong.value().lane2PressLastFrame = true;
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


std::unordered_map<std::string, RawSongData> songToData {};

void loadSong(std::string song){
	if (songToData.find(song) == songToData.end()){
		modassert(false, std::string("could not find song: ") + song);
	}

	unloadSong();

	rhythmSong = RhythmSong {
		.startTime = gameapi -> timeSeconds(false),
		.lastLane0Press = 0.f,
		.lastLane1Press = 0.f,
		.lastLane2Press = 0.f,
		.rawSongData = &songToData.at(song),
		.notes = songToData.at(song).notes,
		.stats = {},
  	.songCurrentHealth = 3,
  	.songTotalHealth = 5,
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
			.noteState = NOTE_DEFAULT,
		});
	}
	rhythmSong.value().stats = stats;

	GameobjAttributes clipAttr {
		.attr = {
			{ "clip", rhythmSong.value().rawSongData->file },
			{ "center", "true" },
		},
	};
	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
	auto clipGameObject = gameapi->makeObjectAttr(
		gameapi->rootSceneId(),
		std::string("&rhythm-song-") + uniqueNameSuffix(),
		clipAttr,
		submodelAttributes
	);
	modassert(clipGameObject.has_value(), "could not create rhythm song clip gameobject");
	rhythmSong.value().clipGameObject = clipGameObject.value();
	rhythmSong.value().playingSong = gameapi->playOneshot(
		clipGameObject.value(),
		std::nullopt,
		std::nullopt,
		std::nullopt,
		false,
		true,
		clipGameObject.value()
	);

}

void unloadSong(){
	if (rhythmSong.has_value()){
		if (rhythmSong.value().playingSong.has_value()){
			gameapi -> stopOneshot(rhythmSong.value().playingSong.value());
		}
		if (rhythmSong.value().clipGameObject.has_value()){
			gameapi -> removeObjectById(rhythmSong.value().clipGameObject.value());
		}
	}
	rhythmSong = std::nullopt;
}

// list songs
RawSongData parseSongData(std::string filepath){
  auto fileInfo = decomposePath(filepath);
  auto fileContent = readFileOrPackage(filepath);

  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());

  modassert(!doc.HasParseError(), "error parsing song file");
  modassert(doc.IsObject(), "song data must be an object");

  modassert(doc.HasMember("name"), "song data missing name");
  modassert(doc["name"].IsString(), "song name must be a string");

  modassert(doc.HasMember("notes"), "song data missing notes");
  modassert(doc["notes"].IsArray(), "song notes must be an array");

  RawSongData result{
    .name = fileInfo.filename,
    .displayName = doc["name"].GetString(),
  };

  modassert(doc.HasMember("file"), "song data missing file");
  modassert(doc["file"].IsString(), "song file must be a string");
  result.file = doc["file"].GetString();

  for (auto& note : doc["notes"].GetArray()){
    modassert(note.IsArray(), "song note must be an array");
    modassert(note.Size() == 2, "song note must contain time and data");

    modassert(note[0].IsNumber(), "song note time must be a number");

    auto& data = note[1];
    modassert(data.IsObject(), "song note data must be an object");

    modassert(data.HasMember("lane"), "song note missing lane");
    modassert(data["lane"].IsInt(), "song note lane must be an integer");

    auto time = note[0].GetFloat();
    auto lane = data["lane"].GetInt();

    modassert(time >= 0.f, "song note time cannot be negative");
    modassert(lane >= 0 && lane <= 2, "song note lane out of range");

    result.notes.push_back(RhythmNote{
      .time = time / 1000.f,
      .lane = lane,
    });
  }

  return result;
}


void loadMidiNotes(std::string midiFile){
 	auto midi = gameapi -> loadMidi(midiFile).value();
 	gameapi -> playMidiFile(midi);


 	for (int i = 0; i < midi.tracks.size(); i++){
 		auto& track = midi.tracks.at(i);
 		for (int j = 0; j < track.events.size(); j++){
 			auto& midiEvent = track.events.at(j);
			//struct MidiEvent {
			//	int midiEventId = 0; // Just for internal uniqueness
			//	uint32_t offset = 0;  // Delta time in MIDI ticks from the previous event
			//	uint64_t absoluteOffset = 0;  // Cumulative MIDI ticks from the start of this track
			//	uint8_t channel = -1;
			//	MidiEventCategory category;
			//	MidiEventType event;
			//};
 			MidiNoteOn* midiNoteOn = std::get_if<MidiNoteOn>(&midiEvent.event);
      auto eventTime = convertEventToTimeSeconds(midi, midiEvent);
      if (midiNoteOn){
      	std::cout << "midi note on time: " << eventTime << ", " << noteNameFromInt(midiNoteOn -> note).value() << std::endl;
      }

 		}
 	}
}

void writeBallConfig(std::string midiFile){
  auto midi = gameapi->loadMidi(midiFile).value();

  // MIDI pitches C4, D4, and E4 map to the three rhythm lanes.
  const std::unordered_map<int, int> noteToLane {
    { 60, 0 },
    { 62, 1 },
    { 64, 2 },
  };

  std::vector<std::pair<float, int>> notes;
  for (auto& track : midi.tracks){
    for (auto& midiEvent : track.events){
      auto* noteOn = std::get_if<MidiNoteOn>(&midiEvent.event);
      if (!noteOn){
        continue;
      }

      auto lane = noteToLane.find(noteOn->note);
      if (lane == noteToLane.end()){
        continue;
      }

      auto eventTime = convertEventToTimeSeconds(midi, midiEvent);
      notes.emplace_back(eventTime * 1000.f, lane->second);
    }
  }
  std::stable_sort(notes.begin(), notes.end(), [](const auto& a, const auto& b){
    return a.first < b.first;
  });

  rapidjson::Document doc;
  doc.SetObject();
  auto& allocator = doc.GetAllocator();
  auto fileInfo = decomposePath(midiFile);
  doc.AddMember("name", rapidjson::Value(fileInfo.filename.c_str(), allocator), allocator);
  doc.AddMember("file", rapidjson::Value(midiFile.c_str(), allocator), allocator);

  rapidjson::Value noteArray(rapidjson::kArrayType);
  for (auto& [time, lane] : notes){
    rapidjson::Value note(rapidjson::kArrayType);
    note.PushBack(time, allocator);
    rapidjson::Value data(rapidjson::kObjectType);
    data.AddMember("lane", lane, allocator);
    note.PushBack(data, allocator);
    noteArray.PushBack(note, allocator);
  }
  doc.AddMember("notes", noteArray, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  std::string json = buffer.GetString();
  std::cout << json << std::endl;
}

std::string songFilenameFromResourcePath(const std::string& resourcePath){
  return decomposePath(resourcePath).filename;
}

bool validateSong(std::string resourcePath, bool isDirectory){
  if (!isDirectory){
    std::cout << "\033[31mError: song resource must be a directory - file = " << resourcePath << "\033[0m" << std::endl;
    return true;
  }

  bool missingFiles = false;
  auto songFiles = listFilesWithExtensionsFromPackage(resourcePath, { "json" });

  for (auto& songFile : songFiles){
    auto songData = parseSongData(songFile);
    auto expectedSongName = songFilenameFromResourcePath(songFile);
    if (songData.name != expectedSongName){
      missingFiles = true;
      std::cout << "\033[31mError: song resource has an unexpected filename - file = " << songFile << "\033[0m" << std::endl;
    }

    if (!fileExistsFromPackage(songData.file)){
      missingFiles = true;
      std::cout << "\033[31mError: song file missing - file = " << songData.file << "\033[0m" << std::endl;
    }
  }

  return missingFiles;
}

std::unordered_map<std::string, RawSongData> loadAllSongs(){
  auto songFiles = listFilesWithExtensionsFromPackage(paths::SONG_DIRECTORY, { "json" });

  std::unordered_map<std::string, RawSongData> songs;
  for (auto& songFile : songFiles){
  	auto songData = parseSongData(songFile);
  	std::cout << "song file: " << songData.name << std::endl;
  	songs[songData.name] = songData;
  }

	return songs;
}

void doLoadAllSongs(){
		songToData = loadAllSongs();
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

int numNotesTotal(RhythmSong& song){
	return song.notes.size();
}
int numNotesSuccess(RhythmSong& song, float currentTime){
	int success = 0;
	for (int i = 0; i < song.notes.size(); i++){
		if (song.stats.at(i).noteState == NOTE_SUCCESS){
			success++;
		}
	}
	return success;
}
int numNotesFailure(RhythmSong& song, float currentTime){
	int failure = 0;
	for (int i = 0; i < song.notes.size(); i++){
		// Probably should allow a buffer here (technically can input after this)
		if (song.stats.at(i).noteState == NOTE_FAILURE){
			failure++;
		}
	}
	return failure;
}

RhythmStats calcRhythmStats(RhythmSong& rhythmSong, float currentPlaybackTime){
	bool songPlaybackComplete = true;
	if (rhythmSong.playingSong.has_value()){
		auto songLength = getSoundLengthOneshot(rhythmSong.playingSong.value());
		if (songLength.has_value()){
	  	std::cout << "song length: " << currentPlaybackTime << " / " << songLength.value() << std::endl;
			songPlaybackComplete =  currentPlaybackTime > songLength.value();
		}else{
			std::cout << "song length: n/a" << std::endl;
		}
	}else{
		std::cout << "song length: no playing song" << std::endl;
	}

  auto totalCount = numNotesTotal(rhythmSong);
  int successCount = numNotesSuccess(rhythmSong, currentPlaybackTime);
  int failureCount = numNotesFailure(rhythmSong, currentPlaybackTime);
  bool songComplete = (successCount + failureCount) == totalCount;
	RhythmStats rhythmStats {
   	.songComplete = songComplete,
   	.songPlaybackComplete = songPlaybackComplete,
  	.numNotesTotal = totalCount,
  	.numSuccess = successCount,
  	.numFailure = failureCount,
  	.songCurrentHealth = rhythmSong.songCurrentHealth,
  	.songTotalHealth = rhythmSong.songTotalHealth,
  };
  return rhythmStats;
}

std::optional<RhythmRendering> getRhythmRendering(float length){
  if (!rhythmSong.has_value()){
    return std::nullopt;
  }
  auto currentTime = gameapi -> timeSeconds(false);

  std::vector<NoteToRender> notesToRender;
  for (int i = 0; i < rhythmSong.value().notes.size(); i++){
		auto& note = rhythmSong.value().notes.at(i);
		auto& stat = rhythmSong.value().stats.at(i);
  	notesToRender.push_back(NoteToRender {
  		.lane = note.lane,
  		.color = stat.noteState == NOTE_SUCCESS ? glm::vec4(0.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f),
  		.time = note.time,
  	});
  }

  float currentPlaybackTime = currentTime - rhythmSong.value().startTime;
  auto rhythmStats = calcRhythmStats(rhythmSong.value(), currentPlaybackTime);

  return RhythmRendering {
    .currentPlaybackTime = currentPlaybackTime,
    .notes = notesToRender,
    .lane0Pressed = isLane0PressedWithGrace(currentTime),
    .lane1Pressed = isLane1PressedWithGrace(currentTime),
    .lane2Pressed = isLane2PressedWithGrace(currentTime),
    .rawSongData = rhythmSong.value().rawSongData,
    .stats = rhythmStats,
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

		bool successNote = false;
		if (note.lane == 0 && isLane0Pressed && stat.noteState == NOTE_DEFAULT){
			stat.noteState = NOTE_SUCCESS;
			successNote = true;
		}
		if (note.lane == 1 && isLane1Pressed && stat.noteState == NOTE_DEFAULT){
			stat.noteState = NOTE_SUCCESS;
			successNote = true;
		}
		if (note.lane == 2 && isLane2Pressed && stat.noteState == NOTE_DEFAULT){
			stat.noteState = NOTE_SUCCESS;
			successNote = true;
		}
		if (successNote){
			rhythmSong.value().songCurrentHealth++;
		}

		if (elapsedTime > (note.time + RHYTHM_GRACE) && stat.noteState == NOTE_DEFAULT){
			std::cout << "set note to failure: " << i << std::endl;
			stat.noteState = NOTE_FAILURE;
			rhythmSong.value().songCurrentHealth--;
			if (rhythmSong.value().songCurrentHealth < 0){
				rhythmSong.value().songCurrentHealth = 0;
			}
		}
	}

	if (rhythmSong.value().lane0PressLastFrame || rhythmSong.value().lane1PressLastFrame || rhythmSong.value().lane2PressLastFrame){
	  	applyUiScreenshakeByPlayerIndex(getDefaultPlayerIndex(), glm::vec2(0.f, 1.f)); // kind of weird to put in here
	}

	rhythmSong.value().lane0PressLastFrame = false;
	rhythmSong.value().lane1PressLastFrame = false;
	rhythmSong.value().lane2PressLastFrame = false;
}

bool isRhythmEnd(){
	if (!rhythmSong.has_value()){
		return false;
	}
	return rhythmSong.value().songCurrentHealth <= 0;
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


void onSongPauseToggle(bool paused){
	if (rhythmSong.has_value() && rhythmSong.value().playingSong.has_value()){
		if (paused){
			std::cout << "rhythm pause: pause oneshot" << std::endl;
			gameapi -> pauseOneshot(rhythmSong.value().playingSong.value());
		}else{
			std::cout << "rhythm pause: resume oneshot" << std::endl;
			gameapi -> resumeOneshot(rhythmSong.value().playingSong.value());
		}
	}
}