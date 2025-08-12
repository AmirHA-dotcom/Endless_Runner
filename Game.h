//
// Created by amirh on 2025-08-09.
//

#ifndef ENDLESS_RUNNER_GAME_H
#define ENDLESS_RUNNER_GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <box2d/box2d.h>

using namespace std;

class Game
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    const char* FONT = "D:/Fonts/Roboto/static/Roboto-Regular.ttf";

public:
    Game();
    void Run();
};


#endif //ENDLESS_RUNNER_GAME_H
