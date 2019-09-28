
#include <mutex>
#include <string>
#include "c_cvars.h"
#include "i_sound.h"
#include "i_music.h"
#include "s_sound.h"
#include "files.h"
#include "zmusic/midiconfig.h"
#include "zmusic/mididefs.h"

#include "zmusic/..//mididevices/mididevice.h"	// this is still needed...

void I_InitMusicWin32 ();

extern float relative_volume;
class MIDISource;
class MIDIDevice;
class OPLmusicFile;


extern ADLConfig adlConfig;
extern FluidConfig fluidConfig;
extern OPLMidiConfig oplMidiConfig;
extern OpnConfig opnConfig;
extern GUSConfig gusConfig;
extern TimidityConfig timidityConfig;
extern WildMidiConfig wildMidiConfig;


class MIDIStreamer;


// Base class for streaming MUS and MIDI files ------------------------------

class MIDIStreamer : public MusInfo
{
public:
	MIDIStreamer(EMidiDevice type, const char *args);
	~MIDIStreamer();

	void MusicVolumeChanged() override;
	void Play(bool looping, int subsong) override;
	void Pause() override;
	void Resume() override;
	void Stop() override;
	bool IsPlaying() override;
	bool IsMIDI() const override;
	bool IsValid() const override;
	bool SetSubsong(int subsong) override;
	void Update() override;
	FString GetStats() override;
	void ChangeSettingInt(const char *setting, int value) override;
	void ChangeSettingNum(const char *setting, double value) override;
	void ChangeSettingString(const char *setting, const char *value) override;
	int ServiceEvent();
	void SetMIDISource(MIDISource *_source);

	int GetDeviceType() const override;

	bool DumpWave(const char *filename, int subsong, int samplerate);
	static bool FillStream(SoundStream* stream, void* buff, int len, void* userdata);


protected:
	MIDIStreamer(const char *dumpname, EMidiDevice type);

	void OutputVolume (uint32_t volume);
	int FillBuffer(int buffer_num, int max_events, uint32_t max_time);
	int FillStopBuffer(int buffer_num);
	uint32_t *WriteStopNotes(uint32_t *events);
	int VolumeControllerChange(int channel, int volume);
	void SetTempo(int new_tempo);
	void Precache();
	void StartPlayback();
	bool InitPlayback();

	//void SetMidiSynth(MIDIDevice *synth);
	
	
	static EMidiDevice SelectMIDIDevice(EMidiDevice devtype);
	MIDIDevice *CreateMIDIDevice(EMidiDevice devtype, int samplerate);

	static void Callback(void *userdata);

	enum
	{
		SONG_MORE,
		SONG_DONE,
		SONG_ERROR
	};

	std::unique_ptr<MIDIDevice> MIDI;
	uint32_t Events[2][MAX_MIDI_EVENTS*3];
	MidiHeader Buffer[2];
	int BufferNum;
	int EndQueued;
	bool VolumeChanged;
	bool Restarting;
	bool InitialPlayback;
	uint32_t NewVolume;
	uint32_t Volume;
	EMidiDevice DeviceType;
	bool CallbackIsThreaded;
	int LoopLimit;
	FString Args;
	std::unique_ptr<MIDISource> source;
	std::unique_ptr<SoundStream> Stream;
};

// Anything supported by the sound system out of the box --------------------

class StreamSong : public MusInfo
{
public:
    StreamSong (FileReader &reader);
	~StreamSong ();
	void Play (bool looping, int subsong);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsValid () const { return m_Stream != NULL; }
	bool SetPosition (unsigned int pos);
	bool SetSubsong (int subsong);
	FString GetStats();

protected:
	StreamSong () : m_Stream(NULL) {}

	SoundStream *m_Stream;
};

// MUS file played by a software OPL2 synth and streamed through the sound system

class OPLMUSSong : public StreamSong
{
public:
	OPLMUSSong (FileReader &reader, const char *args);
	~OPLMUSSong ();
	void Play (bool looping, int subsong);
	bool IsPlaying ();
	bool IsValid () const;
	void ChangeSettingInt (const char *, int) override;

protected:

	static bool FillStream (SoundStream *stream, void *buff, int len, void *userdata);

	OPLmusicFile *Music;
};

// CD track/disk played through the multimedia system -----------------------

class CDSong : public MusInfo
{
public:
	CDSong (int track, int id);
	~CDSong ();
	void Play (bool looping, int subsong);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsValid () const { return m_Inited; }

protected:
	CDSong () : m_Inited(false) {}

	int m_Track;
	bool m_Inited;
};

// CD track on a specific disk played through the multimedia system ---------

class CDDAFile : public CDSong
{
public:
	CDDAFile (FileReader &reader);
};

// Data interface

void Fluid_SetupConfig(FluidConfig *config, const char* patches, bool systemfallback);
void ADL_SetupConfig(ADLConfig *config, const char *Args);
void OPL_SetupConfig(OPLMidiConfig *config, const char *args);
void OPN_SetupConfig(OpnConfig *config, const char *Args);
bool GUS_SetupConfig(GUSConfig *config, const char *args);
bool Timidity_SetupConfig(TimidityConfig* config, const char* args);
bool WildMidi_SetupConfig(WildMidiConfig* config, const char* args);

// Module played via foo_dumb -----------------------------------------------

MusInfo *MOD_OpenSong(FileReader &reader);

// Music played via Game Music Emu ------------------------------------------

const char *GME_CheckFormat(uint32_t header);
MusInfo *GME_OpenSong(FileReader &reader, const char *fmt);
MusInfo *SndFile_OpenSong(FileReader &fr);
MusInfo* XA_OpenSong(FileReader& reader);

// --------------------------------------------------------------------------

extern MusInfo *currSong;
void MIDIDeviceChanged(int newdev, bool force = false);

EXTERN_CVAR (Float, snd_mastervolume)
EXTERN_CVAR (Float, snd_musicvolume)
