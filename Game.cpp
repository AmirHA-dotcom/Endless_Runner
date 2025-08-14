//
// Created by amirh on 2025-08-09.
//

#include "Game.h"

Game::Game()
{
    srand(time(0));

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
        Update_Ground();

        // Camara
        b2Vec2 playerPosMeters = m_Player->get_position();
        float playerPosPixelsX = playerPosMeters.x * PIXELS_PER_METER;

        // The camera's position is its top-left corner
        cameraX = playerPosPixelsX - (SCREEN_WIDTH / 2.0f);

        // Rendering
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        m_Player->Render(renderer, cameraX);

        for (const auto& segment : m_Ground_Segments)
        {
            segment->Render(renderer, cameraX);
        }
        for (const auto& obstacle : m_Obstacles)
        {
            obstacle->Render(renderer, cameraX);
        }

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

void Game::Update_Ground()
{
    // If the right edge of the last ground segment is on screen, add a new one.
    Scenery* lastSegment = m_Ground_Segments.back().get();
    if (lastSegment->Get_Right_EdgeX() < cameraX + SCREEN_WIDTH + 200) // +200 for a buffer
    {
        float nextX = lastSegment->Get_Right_EdgeX();
        m_Ground_Segments.push_back(make_unique<Scenery>(World_Id, nextX));

        // Give a 1 in 4 (25%) chance to spawn an obstacle on a new segment
        if (rand() % 4 == 0)
        {
            // Calculate spawn position on top of the new ground segment
            float groundSurfaceY = SCREEN_HEIGHT - 40.0f; // 40 is the ground height
            float obstacleHeight = 50.0f; // As defined in your Obstacle constructor

            float spawnX = nextX + SCREEN_WIDTH / 2.0f; // Center of the new segment
            float spawnY = groundSurfaceY - (obstacleHeight / 2.0f);

            m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY));
        }
    }

    // If the right edge of the first ground segment is off the left side of the screen, remove it.
    Scenery* firstSegment = m_Ground_Segments.front().get();
    if (firstSegment->Get_Right_EdgeX() < cameraX)
    {
        m_Ground_Segments.pop_front();
    }
    if (!m_Obstacles.empty() && m_Obstacles.front()->Get_Right_EdgeX() < cameraX)
    {
        m_Obstacles.pop_front();
    }
}