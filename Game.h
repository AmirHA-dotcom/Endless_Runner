//
// Created by amirh on 2025-08-09.
//

#ifndef ENDLESS_RUNNER_GAME_H
#define ENDLESS_RUNNER_GAME_H


#include "Object.h"

using namespace std;

class Game
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    b2WorldId World_Id;
    const char* FONT = "D:/Fonts/Roboto/static/Roboto-Regular.ttf";
    const float M2P = 50.0f;
    const float P2M = 1.0f / M2P;

    unique_ptr<Player> Player;
    unique_ptr<Scenery> Ground;
public:
    Game();
    void Run();
};


#endif //ENDLESS_RUNNER_GAME_H
