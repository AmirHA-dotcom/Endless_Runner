//
// Created by amirh on 2025-08-14.
//

#ifndef ENDLESS_RUNNER_AUDIO_MANAGER_H
#define ENDLESS_RUNNER_AUDIO_MANAGER_H

#include <box2d/box2d.h>
#include "box2d/collision.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <SDL2/SDL_mixer.h>
#include "map"

using namespace std;

class Audio_Manager
{
private:
    Audio_Manager() {}
    ~Audio_Manager() {}

    map<string, Mix_Chunk*> m_sounds;
public:
    // Gets the single instance of the AudioManager
    static Audio_Manager& GetInstance()
    {
        static Audio_Manager instance;
        return instance;
    }

    // Call this once at the start of the game
    void Init();

    // Load a sound effect and give it a name
    void LoadSound(const string& name, const string& path);

    // Play a loaded sound effect by name
    void PlaySound(const string& name);

    // Call this once at the end of the game
    void CleanUp();
};


#endif //ENDLESS_RUNNER_AUDIO_MANAGER_H
