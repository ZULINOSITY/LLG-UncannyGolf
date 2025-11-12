#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <map>

struct AudioClip {
    Uint8* buffer;
    Uint32 length;
    SDL_AudioSpec spec;
};

class AudioHandler {
public:
    static AudioHandler& getInstance();
    
    bool initialize();
    void shutdown();
    
    bool loadWAV(const std::string& filename, const std::string& id); // Carga los archivos
    void playSound(const std::string& id); // Reproduce el archivo mediante su id registrada
    void stopAllSounds();
    void printAudioInfo(); // Diagnostico de archivos
    
    void setVolume(float volume);
    float getVolume() const;

    bool isInitialized() const { return initialized; } // (Diagnostico) Comprueba si se encuentra inicializado el sistema

private:
    AudioHandler();
    ~AudioHandler();
    AudioHandler(const AudioHandler&) = delete;
    AudioHandler& operator=(const AudioHandler&) = delete;
    
    static void audioCallback(void* userdata, Uint8* stream, int len);
    void mixAudio(Uint8* stream, int len);
    
    SDL_AudioDeviceID audioDevice;
    std::map<std::string, AudioClip> audioClips;
    std::vector<std::pair<const AudioClip*, Uint32>> playingSounds; // clip + posición
    
    float volume;
    bool initialized;
};

#endif