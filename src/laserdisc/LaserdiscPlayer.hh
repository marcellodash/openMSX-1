// $Id$

#ifndef LASERDISCPLAYER_HH
#define LASERDISCPLAYER_HH

#include "SoundDevice.hh"
#include "Resample.hh"
#include "EmuTime.hh"
#include "Schedulable.hh"
#include "DynamicClock.hh"
#include "Clock.hh"
#include "VideoSystemChangeListener.hh"

namespace openmsx {

class LaserdiscCommand;
class PioneerLDControl;
class MSXMotherBoard;
class OggReader;
class AudioFragment;
class LDRenderer;

class LaserdiscPlayer : public SoundDevice
		      , public Schedulable
		      , private Resample
		      , private VideoSystemChangeListener
{
public:
	LaserdiscPlayer(MSXMotherBoard& motherBoard, PioneerLDControl& ldcontrol);
	~LaserdiscPlayer();

	// Called from CassettePort
	short readSample(EmuTime::param time);

	// Called from PioneerLDControl
	void setMuting(bool left, bool right, EmuTime::param time);
	bool extAck(EmuTime::param time) const;
	void extControl(bool bit, EmuTime::param time);

	// video interface
	MSXMotherBoard& getMotherBoard() { return motherBoard; }

private:
	void setImageName(const std::string& newImage, EmuTime::param time);

	/** Laserdisc player commands
	  */
	void play(EmuTime::param time);
	void pause(EmuTime::param time);
	void stop(EmuTime::param time);
	void seekFrame(int frame, EmuTime::param time);
	void seekChapter(int chapter, EmuTime::param time);

	// Control from MSX

	/** Is video output being generated?
	  */
	bool isVideoOutputAvailable(EmuTime::param time);
	bool extInt(EmuTime::param time);
	void button(unsigned custom, unsigned code, EmuTime::param time);
	void buttonRepeat(EmuTime::param time);
	void setAck(EmuTime::param time, int wait);
	unsigned getCurrentSample(EmuTime::param time);
	void createRenderer();

	// SoundDevice
	void setOutputRate(unsigned sampleRate);
	void generateChannels(int** bufs, unsigned num);
	bool updateBuffer(unsigned length, int* buffer,
	                  EmuTime::param time, EmuDuration::param sampDur);
	bool generateInput(int* buffer, unsigned num);

	// Schedulable
	const std::string& schedName() const;
	void executeUntil(EmuTime::param time, int userData);

	// VideoSystemChangeListener interface:
	void preVideoSystemChange();
	void postVideoSystemChange();

	MSXMotherBoard& motherBoard;
	PioneerLDControl& ldcontrol;

	const std::auto_ptr<LaserdiscCommand> laserdiscCommand;
	std::auto_ptr<OggReader> video;
	std::auto_ptr<LDRenderer> renderer;

	// Audio state
	DynamicClock sampleClock;
	EmuTime start;
	unsigned outputRate;
	unsigned playingFromSample;
	unsigned lastPlayedSample;
	bool muteLeft, muteRight;

	Clock<30000, 1001> frameClock;

	enum SyncType {
		FRAME,
		ACK
	};

	// Ext Control
	enum RemoteState {
		REMOTE_GAP,
		REMOTE_HEADER_PULSE,
		REMOTE_HEADER_SPACE,
		REMOTE_BITS_PULSE,
		REMOTE_BITS_SPACE,
		REMOTE_REPEAT_PULSE
	} remoteState;
	EmuTime remoteLastEdge;
	unsigned remoteBitNr;
	unsigned remoteBits;
	bool remoteLastBit;

	/* We need to maintain some state for seeking */
	enum PioneerSeekState {
		SEEK_NONE,
		SEEK_CHAPTER_BEGIN,
		SEEK_CHAPTER_END,
		SEEK_FRAME_BEGIN,
		SEEK_FRAME_END
	} seekState;

	/* The specific frame or chapter we are seeking to */
	unsigned seekNum;

	// For ack
	bool ack;

	// State of the video itself
	bool seeking;

	enum PlayerState {
		PLAYER_STOPPED,
		PLAYER_PLAYING,
		PLAYER_PAUSED,
		PLAYER_FROZEN
	} playerState;

	friend class LaserdiscCommand;
};

} // namespace openmsx

#endif
