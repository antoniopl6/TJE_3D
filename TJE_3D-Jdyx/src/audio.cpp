#include "audio.h"

Audio* Audio::Get(const char* filename) {

	HSAMPLE sample = BASS_SampleLoad(false, filename, 0, 0, 3, 0);
	if (sample == 0) {
		std::cout << "ERROR loading file " << filename << std::endl;
	}
};

HCHANNEL* Audio::Play(const char* filename) {
	Audio* audio = Get(filename);
	HSAMPLE hSample = audio->sample;

	HCHANNEL hSampleChannel;
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	BASS_ChannelPlay(hSampleChannel, true);
};

void Audio::Stop(HCHANNEL channel) {
	if (!BASS_ChannelStop(channel))
		std::cout << "ERROR stopping channel audio " << std::endl;
}