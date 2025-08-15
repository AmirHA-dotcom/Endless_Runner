//
// Created by amirh on 2025-08-15.
//

#ifndef ENDLESS_RUNNER_ASSET_MANAGER_H
#define ENDLESS_RUNNER_ASSET_MANAGER_H

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

class Asset_Manager
{
private:
    Asset_Manager() {}
    ~Asset_Manager() {}
    map<string, SDL_Texture*> m_textures;
public:
    static Asset_Manager& GetInstance()
    {
        static Asset_Manager instance;
        return instance;
    }

    // Load a texture from a file and store it by name
    void LoadTexture(const string& name, const std::string& path, SDL_Renderer* renderer);

    // Get a previously loaded texture by name
    SDL_Texture* GetTexture(const string& name);

    // Free all loaded textures
    void CleanUp();
};


#endif //ENDLESS_RUNNER_ASSET_MANAGER_H
