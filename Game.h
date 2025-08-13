//
// Created by amirh on 2025-08-09.
//

#ifndef ENDLESS_RUNNER_GAME_H
#define ENDLESS_RUNNER_GAME_H


#include "Object.h"

class Game
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    b2WorldId World_Id;
    const char* FONT = "D:/Fonts/Roboto/static/Roboto-Regular.ttf";
    const float PIXELS_PER_METER = 30.0f;

    float cameraX = 0.0f;

    unique_ptr<Player> m_Player;
    //unique_ptr<Scenery> m_Ground;
    deque<unique_ptr<Scenery>> m_Ground_Segments;
public:
    Game();
    void Run();

    void Generate_Initial_Ground();
    void Update_Ground();
};


#endif //ENDLESS_RUNNER_GAME_H
