#ifndef AUDIO_H
#define AUDIO_H
#include "includes.h"
#include "utils.h"
#include <map>
#include <string>
#include "extra/bass.h"

class Audio
{
public:
	static std::map<std::string, Audio*> sLoadedAudios; //para nuestro manager
	HSAMPLE sample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad

	Audio() {
		assert(BASS_Init(-1, 44100, 0, 0, NULL), "Error opening sound card."); //-1 significa usar el por defecto del sistema operativo
		sample = 0;
	};
	~Audio() {
		BASS_SampleFree(sample);
	};

	HCHANNEL play(float volume); //lanza el audio y retorna el channel donde suena

	static void Stop(HCHANNEL channel); //para parar un audio necesitamos su channel
	static Audio* Get(const char* filename); //manager de audios 
	static HCHANNEL* Play(const char* filename); //version estática para ir mas rapido
};
#endif