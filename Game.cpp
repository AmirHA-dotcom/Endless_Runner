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
    worldDef.gravity = {0.0f, 40.0f};
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

        // FPS
        float timeStep = 1.0f / 60.0f;
        b2World_Step(World_Id, timeStep, 3);

        // Updating
        m_Player->Update(World_Id);
        Update_Ground();
        Update_Spawning(timeStep);

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
                        if (m_Player->Can_Jump())
                        {
                            m_Player->Jump();
                        }
                    }
                }
            }
        }

        // Check if the Game is Over
        if (m_Player->IsDead())
        {
            cout << "Game Over!" << std::endl;
            Running = false;
        }

        // Collision
        if (!m_Player->IsDead())
        {
            b2Vec2 playerCenter = m_Player->get_position();
            float playerRadius = m_Player->Get_Radius_Meters();

            for (const auto& obstacle : m_Obstacles)
            {
                b2Vec2 obstacleCenter = obstacle->get_position();
                float obstacleHalfWidth = obstacle->GetWidthMeters() / 2.0f;
                float obstacleHalfHeight = obstacle->GetHeightMeters() / 2.0f;

                float closestX = max(obstacleCenter.x - obstacleHalfWidth, min(playerCenter.x, obstacleCenter.x + obstacleHalfWidth));
                float closestY = max(obstacleCenter.y - obstacleHalfHeight, min(playerCenter.y, obstacleCenter.y + obstacleHalfHeight));

                float deltaX = playerCenter.x - closestX;
                float deltaY = playerCenter.y - closestY;
                float distanceSq = (deltaX * deltaX) + (deltaY * deltaY);

                if (distanceSq < (playerRadius * playerRadius))
                {
                    m_Player->SetIsDead(true);
                    cout << "Collision Detected! Game Over." << std::endl;
                    break;
                }
            }
        }

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

void Game::Update_Spawning(float deltaTime)
{
    // Decrease the timer
    m_Obstacle_Spawn_Timer -= deltaTime;

    // If the timer has run out, it's time to spawn a new obstacle
    if (m_Obstacle_Spawn_Timer <= 0.0f)
    {
        const float groundSurfaceY = SCREEN_HEIGHT - 40.0f; // 40px ground height
        float spawnX = cameraX + SCREEN_WIDTH + 100; // Spawn 100px off-screen to the right

        // Randomly choose an obstacle type to spawn
        int obstacleType = rand() % 3;

        switch (obstacleType)
        {
            case 0: // Short Obstacle (jump over)
            {
                float width = 50.0f;
                float height = 50.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
            case 1: // Tall Obstacle (requires double jump)
            {
                float width = 50.0f;
                float height = 150.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
            case 2: // Wide Obstacle (requires precise jump timing)
            {
                float width = 200.0f;
                float height = 50.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
        }

        float randomTime = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.5f));
        m_Obstacle_Spawn_Timer = randomTime;
    }
}