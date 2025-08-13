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

    window = SDL_CreateWindow(
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


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw runtime_error("Renderer could not be created! SDL_Error: " + string(SDL_GetError()));
    }

    // box2d initializations
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 20.0f};
    World_Id = b2CreateWorld(&worldDef);

    // Window size
    SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Objects
    m_Player = make_unique<Player>(World_Id);
    //m_Ground = make_unique<Scenery>(World_Id);
    Generate_Initial_Ground();

}

void Game::Run()
{
    // variables
    bool Running = true;
    SDL_Event event;
    const int TARGET_FPS = 60;
    const int FRAME_DELAY = 1000 / TARGET_FPS;
    Uint32 frameStart;
    int frameTime;

    // game loop
    while (Running)
    {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                Running = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    {
                        Running = false;
                    }
                    case SDLK_SPACE:
                    {
                        m_Player->Jump();
                    }
                }
            }
        }
        // FPS
        float timeStep = 1.0f / 60.0f;
        b2World_Step(World_Id, timeStep, 3);

        // Updating
        m_Player->Update();
        //m_Ground->Update();

        // Camara
        b2Vec2 playerPosMeters = m_Player->get_position();
        float playerPosPixelsX = playerPosMeters.x * PIXELS_PER_METER;

        // The camera's position is its top-left corner
        cameraX = playerPosPixelsX - (SCREEN_WIDTH / 2.0f);

        // Rendering
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        //m_Ground->Render(renderer, cameraX);
        m_Player->Render(renderer, cameraX);

        SDL_RenderPresent(renderer);

        // FPS
        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

}

void Game::Generate_Initial_Ground()
{
    float currentX = 0.0f;
    // Create enough segments to fill about two screens worth of ground
    for (int i = 0; i < 3; ++i)
    {
        m_Ground_Segments.push_back(make_unique<Scenery>(World_Id, currentX));
        // The next segment will start at the right edge of this one
        currentX = m_Ground_Segments.back()->Get_Right_EdgeX();
    }
}