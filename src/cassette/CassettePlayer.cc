// $Id$

#include <stdlib.h>
#include "CassettePlayer.hh"
#include "PluggingController.hh"
#include "CommandController.hh"
#include "FileOpener.hh"


CassettePlayer::CassettePlayer()
{
	motor = false;
	audioLength = 0;	// no tape inserted (yet)
	try {
		MSXConfig::Config *config =
			MSXConfig::Backend::instance()->getConfigById("tape");
		std::string filename = config->getParameter("filename");
		insertTape(filename);
	} catch (MSXException& e) {
		PRT_DEBUG("No correct tape insertion!");
	}
	PluggingController::instance()->registerPluggable(this);
	CommandController::instance()->registerCommand(*this, "tape");
}

CassettePlayer::~CassettePlayer()
{
	CommandController::instance()->unregisterCommand("tape");
	PluggingController::instance()->unregisterPluggable(this);
	removeTape();	// free memory
}

void CassettePlayer::insertTape(const std::string &filename)
{
	// TODO throw exceptions instead of PRT_ERROR
	const char* file = FileOpener::findFileName(filename).c_str();
	if (audioLength != 0)
		removeTape();
	if (SDL_LoadWAV(file, &audioSpec, &audioBuffer, &audioLength) == NULL)
		PRT_ERROR("CassettePlayer error: " << SDL_GetError());
	if (audioSpec.format != AUDIO_S16)
		PRT_ERROR("CassettePlayer error: unsupported WAV format");
	posReference = 0;	// rewind tape (make configurable??)
}

void CassettePlayer::removeTape()
{
	SDL_FreeWAV(audioBuffer);
	audioLength = 0;
}

int CassettePlayer::calcSamples(const EmuTime &time)
{
	float duration = (time - timeReference).toFloat();
	int samples = (int)(duration*audioSpec.freq);
	return posReference + samples;
}

void CassettePlayer::setMotor(bool status, const EmuTime &time)
{
	if (motor != status) {
		motor = status;
		if (motor) {
			// motor turned on
			PRT_DEBUG("CasPlayer motor on");
			timeReference = time;
		} else {
			// motor turned off
			PRT_DEBUG("CasPlayer motor off");
			posReference = calcSamples(time);
		}
	}
}

short CassettePlayer::readSample(const EmuTime &time)
{
	int samp;
	if (motor) {
		// motor on
		Uint32 index = calcSamples(time);
		if (index < (audioLength/2)) {
			samp =  ((short*)audioBuffer)[index];
		} else {
			samp = 0;
		}
	} else {
		// motor off
		samp = 0;
	}
	//PRT_DEBUG("CasPlayer read "<<(int)samp);
	return samp;
}

void CassettePlayer::writeWave(short *buf, int length)
{
	// recording not implemented yet
}

int CassettePlayer::getWriteSampleRate()
{
	// recording not implemented yet
	return 0;	// 0 -> not interested in writeWave() data
}


const std::string &CassettePlayer::getName()
{
	static const std::string name("cassetteplayer");
	return name;
}


void CassettePlayer::execute(const std::vector<std::string> &tokens)
{
	if (tokens.size() != 2)
		throw CommandException("Syntax error");
	if (tokens[1] == "eject") {
		print("Tape ejected");
		removeTape();
	} else {
		print("Changing tape");
		insertTape(tokens[1]);
	}
}

void CassettePlayer::help(const std::vector<std::string> &tokens)
{
	print("tape eject      : remove tape from virtual player");
	print("tape <filename> : change the tape file");
}

void CassettePlayer::tabCompletion(std::vector<std::string> &tokens)
{
	if (tokens.size() == 2)
		CommandController::completeFileName(tokens);
}
