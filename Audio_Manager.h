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
#include <SDL2/SDL_image.h>
#include "map"

using namespace std;

class Audio_Manager
{
private:
    Audio_Manager() {}
    ~Audio_Manager() {}

    map<string, Mix_Chunk*> m_sounds;
public:
    static Audio_Manager& GetInstance()
    {
        static Audio_Manager instance;
        return instance;
    }
    void Init();
    void LoadSound(const string& name, const string& path);
    void PlaySound(const string& name);
    void CleanUp();
};


#endif //ENDLESS_RUNNER_AUDIO_MANAGER_H
