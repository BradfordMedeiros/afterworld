#include "./midi.h"

#pragma pack(push, 1)
struct MidiHeader {
    char magic[4];       // MThd
    uint32_t length = 0;     // 6
    uint16_t format = 0;
    uint16_t trackCount = 0;
    uint16_t division = 0;
};
#pragma pack(pop)

std::string print(MidiHeader& midiHeader){
	std::string value;
	value += std::string("midiheader = [") + "\n";
	value += "  magic = " + std::string(midiHeader.magic, 4) + "\n";
	value += "  length = " + std::to_string(midiHeader.length) + "\n";
	value += "  format = " + std::to_string(midiHeader.format) + "\n";
	value += "  trackCount = " + std::to_string(midiHeader.trackCount) + "\n";
	value += "  division = " + std::to_string(midiHeader.division) + "\n";
	value += "]\n";
	return value;
}

#pragma pack(push, 1)
struct MidiTrack {
	char magic[4];  // Mtrk
	uint32_t length = 0;   
};
#pragma pack(pop)

std::string print(MidiTrack& midiTrack){
	std::string value;
	value += std::string("miditrack = [") + "\n";
	value += "  magic = " + std::string(midiTrack.magic, 4) + "\n";
	value += "  length = " + std::to_string(midiTrack.length) + "\n";
	value += "]\n";
	return value;
}


uint32_t readMidiVariableLength(FILE* file){
    uint32_t value = 0;
    while (true){
        uint8_t byte = 0;
        fread(&byte, 1, 1, file);

        value = (value << 7) | (byte & 0x7F);

        if ((byte & 0x80) == 0){
            break;
        }
    }
    return value;
}

void loadMidi(std::string filename){

	MidiHeader midiHeader{};
	FILE* file = fopen(filename.c_str(), "rb");
	modassert(file, "file handle is null loadMidi");

	fread(&midiHeader, sizeof(midiHeader), 1, file);

	static_assert(sizeof(MidiHeader) == 14); // if this gets hit the compiler has a padding issue

	midiHeader.length = ntohl(midiHeader.length);
	midiHeader.format = ntohs(midiHeader.format);
	midiHeader.trackCount = ntohs(midiHeader.trackCount);
	midiHeader.division = ntohs(midiHeader.division);

	std::cout << print(midiHeader) << std::endl;

	modassert(midiHeader.magic[0] == 'M' && midiHeader.magic[1] == 'T' && midiHeader.magic[2] == 'h' && midiHeader.magic[3] == 'd', std::string("Unexpected midiHeader - probably not a midi file: ") + std::string(midiHeader.magic, 4));;
	modassert(midiHeader.length == 6, "midi expected length 6");  

	for (int i = 0; i < midiHeader.trackCount; i++){
		MidiTrack midiTrack{};

		static_assert(sizeof(MidiTrack) == 8); // if this gets hit the compiler has a padding issue
		fread(&midiTrack, sizeof(midiTrack), 1, file);

		midiTrack.length = ntohl(midiTrack.length);

		modassert(midiTrack.magic[0] == 'M' && midiTrack.magic[1] == 'T' && midiTrack.magic[2] == 'r' && midiTrack.magic[3] == 'k', std::string("Unexpected midiTrack - probably not a midi file: ") + std::string(midiTrack.magic, 4));;

		//fseek(file, midiTrack.length, SEEK_CUR);

		long trackStart = ftell(file);
		long trackCurrent = trackStart;

		uint8_t runningStatus = 0;

		int numberOfEvents = 0;
		while (trackCurrent - trackStart < midiTrack.length){
    		// Read the event

			numberOfEvents++;

    		auto deltaTime = readMidiVariableLength(file);

    		uint8_t statusByte = 0;
    		modassert(fread(&statusByte, 1, 1, file) == 1,  "Failed to read MIDI status byte");

    		uint8_t firstDataByte = 0;
    		bool hasFirstDataByte = false;


    		if (statusByte >= 0x80 && statusByte <= 0xEF){
    		    runningStatus = statusByte;
    		}else if (statusByte < 0x80){
    		    modassert(runningStatus != 0, "Invalid MIDI running status");
		
    		    firstDataByte = statusByte;
    		    hasFirstDataByte = true;
    		    statusByte = runningStatus;
    		}

    		if (statusByte >= 0x80 && statusByte <= 0xEF){
    		    // Channel event
    		    uint8_t eventType = statusByte & 0xF0;
    		    uint8_t channel = statusByte & 0x0F;
    		    if (eventType == 0x80){
    		        // Note Off
    		        uint8_t note = firstDataByte;
    		        uint8_t velocity;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&velocity, 1, 1, file);
    		    }else if (eventType == 0x90){
    		        // Note On
    		        uint8_t note = firstDataByte;
    		        uint8_t velocity;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&velocity, 1, 1, file);
    		    }else if (eventType == 0xA0){
    		        // Polyphonic Aftertouch
    		        uint8_t note = firstDataByte;
    		        uint8_t pressure;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&pressure, 1, 1, file);
    		    }else if (eventType == 0xB0){
    		        // Control Change
    		        uint8_t controller = firstDataByte;
    		        uint8_t value;
    		        if (!hasFirstDataByte) fread(&controller, 1, 1, file);
    		        fread(&value, 1, 1, file);
    		    }else if (eventType == 0xC0){
    		        // Program Change
    		        uint8_t program = firstDataByte;
    		        if (!hasFirstDataByte) fread(&program, 1, 1, file);
    		    }else if (eventType == 0xD0){
    		        // Channel Aftertouch
    		        uint8_t pressure = firstDataByte;
    		        if (!hasFirstDataByte) fread(&pressure, 1, 1, file);
    		    }else if (eventType == 0xE0){
    		        // Pitch Bend
    		        uint8_t lsb = firstDataByte;
    		        uint8_t msb;
    		        if (!hasFirstDataByte) fread(&lsb, 1, 1, file);
    		        fread(&msb, 1, 1, file);
    		    }else{
    		        modassert(false, "default case channel error");
    		    }
    		}else if (statusByte == 0xFF){
    		    // Meta event
    		    // just skips past it for now.  
 
    			// Meta event
    			runningStatus = 0;

    			uint8_t metaType = 0;
    			fread(&metaType, 1, 1, file);

    			uint32_t metaLength = readMidiVariableLength(file);

    			std::vector<uint8_t> metaData(metaLength);
    			fread(metaData.data(), 1, metaLength, file);

    		}else if (statusByte == 0xF0 || statusByte == 0xF7){
    		    // SysEx event
    		    runningStatus = 0;

    			uint32_t sysexLength = readMidiVariableLength(file);

		    	fseek(file, sysexLength, SEEK_CUR);
    		}else{
    		    modassert(false, "Unknown MIDI event status");
    		}

    		trackCurrent = ftell(file);
    		modassert(trackCurrent - trackStart <= midiTrack.length, "MIDI track read past declared length");
		}

		std::cout << print(midiTrack) << std::endl;
		std::cout << "number of events: " << numberOfEvents << std::endl;

	}


	modassert(false, "loadMidi not yet implemented");
}