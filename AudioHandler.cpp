#include "AudioHandler.h"
#include <iostream>
#include <algorithm>

AudioHandler::AudioHandler() 
    : audioDevice(0), volume(1.0f), initialized(false) {
}

AudioHandler::~AudioHandler() {
    shutdown();
}

AudioHandler& AudioHandler::getInstance() {
    static AudioHandler instance;
    return instance;
}

bool AudioHandler::initialize() {
    if (initialized) {
        return true;
    }
    
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL audio initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_AudioSpec desiredSpec, obtainedSpec;
    
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S16LSB;
    desiredSpec.channels = 2;
    desiredSpec.samples = 4096;
    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = this;
    
     audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, nullptr, 0);
    
    if (audioDevice == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Start audio playback
    SDL_PauseAudioDevice(audioDevice, 0);
    initialized = true;
    
    return true;
}

void AudioHandler::shutdown() {
    if (!initialized) return;
    
    SDL_PauseAudioDevice(audioDevice, 1);
    SDL_CloseAudioDevice(audioDevice);
    
    // Free all audio buffers
    for (auto& pair : audioClips) {
        SDL_FreeWAV(pair.second.buffer);
    }
    audioClips.clear();
    playingSounds.clear();
    
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    initialized = false;
}

bool AudioHandler::loadWAV(const std::string& filename, const std::string& id) {
    if (!initialized) {
        std::cerr << "AudioHandler not initialized" << std::endl;
        return false;
    }
    
    AudioClip clip;
    SDL_zero(clip);
    
    if (SDL_LoadWAV(filename.c_str(), &clip.spec, &clip.buffer, &clip.length) == nullptr) {
        std::cerr << "Failed to load WAV file: " << filename << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    audioClips[id] = clip;
    return true;
}

void AudioHandler::playSound(const std::string& id) {
    if (!initialized) return;
    
    auto it = audioClips.find(id);
    if (it != audioClips.end()) {
        SDL_LockAudioDevice(audioDevice);
        playingSounds.push_back({&(it->second), 0}); // posición inicial 0
        SDL_UnlockAudioDevice(audioDevice);
    }
}

void AudioHandler::stopAllSounds() {
    if (!initialized) return;
    
    SDL_LockAudioDevice(audioDevice);
    playingSounds.clear();
    SDL_UnlockAudioDevice(audioDevice);
}

void AudioHandler::setVolume(float volume) {
    this->volume = std::max(0.0f, std::min(1.0f, volume));
}

float AudioHandler::getVolume() const {
    return volume;
}

void AudioHandler::audioCallback(void* userdata, Uint8* stream, int len) {
    AudioHandler* handler = static_cast<AudioHandler*>(userdata);
    handler->mixAudio(stream, len);
}
void AudioHandler::mixAudio(Uint8* stream, int len) {
    SDL_memset(stream, 0, len);
    
    if (playingSounds.empty()) return;
    
    for (auto it = playingSounds.begin(); it != playingSounds.end();) {
        const AudioClip* clip = it->first;
        Uint32& position = it->second;
        
        Uint32 remaining = clip->length - position;
        if (remaining == 0) {
            it = playingSounds.erase(it);
            continue;
        }
        
        Uint32 mixLength = (remaining < static_cast<Uint32>(len)) ? remaining : len;
        
        if (mixLength > 0) {
            SDL_MixAudioFormat(stream, 
                              clip->buffer + position, 
                              AUDIO_S16, 
                              mixLength, 
                              static_cast<int>(volume * SDL_MIX_MAXVOLUME));
            
            position += mixLength;
        }
        
        if (position >= clip->length) {
            it = playingSounds.erase(it);
        } else {
            ++it;
        }
    }
}