#include <cmath>
#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL/SDL.h>

#define D2R 0.0174532925

ALuint buf_wave[1], src_wave[1];

int main(int argc, char* argv[]) {
	ALCdevice* dev = alcOpenDevice(NULL);
	if (dev == NULL) {
		std::cerr << "[erro] alcOpenDevice" << std::endl;
		exit(1);
	}
	ALCcontext* context = alcCreateContext(dev, NULL);
	alcMakeContextCurrent(context);
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "[erro] SDL_Init" << std::endl;
		exit(1);
	}
	
	alGetError(); // suave na nave
	alGenBuffers(1, buf_wave);
	if (alGetError() != AL_NO_ERROR) {
		std::cerr << "[erro] alGenBuffers" << std::endl;
		exit(1);
	}
	uint8_t* wave;
	int samples, frequency;
	std::cout << "Which frequency? ";
	std::cin >> frequency;
	std::cout << "How many samples for the wave? [try me between 2 and 16] ";
	std::cin >> samples;
	ALsizei freq = samples * frequency;
	ALsizei size = freq;
	if (!(wave = new uint8_t[size]))
		return -2;
	
	for (int i = 0; i < size; ++i) {
		float x = i * 360.0 / (float)samples;
		wave[i] = sin(x * D2R) * 128 + 128;
	}
	ALvoid* data = wave;
	alBufferData(buf_wave[0], AL_FORMAT_MONO8, data, size, freq);
	alGenSources(1, src_wave);
	if (alGetError() != AL_NO_ERROR) {
		std::cerr << "[erro] alGenSources" << std::endl;
		exit(1);
	}
	alSourcei(src_wave[0], AL_BUFFER, buf_wave[0]);
	alSourcei(src_wave[0], AL_LOOPING, AL_TRUE);
	alSourcePlay(src_wave[0]);
	ALfloat pitch = 1.0, gain = 1.0;
	SDL_SetVideoMode(400, 300, 16, SDL_SWSURFACE);
	SDL_Event e;
	while (true) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				alSourceStop(src_wave[0]);
				alcMakeContextCurrent(NULL);
				alcDestroyContext(context);
				alcCloseDevice(dev);
				SDL_Quit();
				return 0;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					pitch += 0.1;
					alSourcef(src_wave[0], AL_PITCH, pitch);
					break;
				case SDLK_DOWN:
					pitch -= 0.1;
					alSourcef(src_wave[0], AL_PITCH, pitch);
					break;
				case SDLK_LEFT:
					gain -= 0.1;
					alSourcef(src_wave[0], AL_GAIN, gain);
					break;
				case SDLK_RIGHT:
					gain += 0.1;
					alSourcef(src_wave[0], AL_GAIN, gain);
					break;
				default: break;
				}
				break;
			default: break;
			}
		}
	}
	return 0;
}
