//
// Created by amirh on 2025-08-09.
//

#include "Game.h"

Game::Game()
{
    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        throw runtime_error("SDL could not initialize! SDL_Error: " + string(SDL_GetError()));
    }

    if (TTF_Init() == -1)
    {
        SDL_Quit();
        throw runtime_error("SDL_ttf could not initialize! TTF_Error: " + string(TTF_GetError()));
    }

    TTF_Font* font = TTF_OpenFont(FONT , 16);
    if (!font)
    {
        cerr << "Failed to load font: " << TTF_GetError() << endl;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Endless Runner",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1280,
            720,
            SDL_WINDOW_FULLSCREEN_DESKTOP
    );


    if (!window)
    {
        SDL_Quit();
        throw runtime_error("Window could not be created! SDL_Error: " + string(SDL_GetError()));
    }


    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw runtime_error("Renderer could not be created! SDL_Error: " + string(SDL_GetError()));
    }

    // box2d initializations
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 20.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

}

void Game::Run()
{
    // variables
    bool Running = true;
    SDL_Event event;

    // game loop
    while (Running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                Running = false;
            }
        }
    }

}