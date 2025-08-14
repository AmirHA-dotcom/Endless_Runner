//
// Created by amirh on 2025-08-09.
//

#include "Game.h"

// helper

inline void render_text(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y, SDL_Color color = {0, 0, 0, 255})
{
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest_rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Functions

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

    font_large = TTF_OpenFont(FONT , 48);
    if (!font_large)
    {
        cerr << "Failed to load font_large: " << TTF_GetError() << endl;
    }

    font_regular = TTF_OpenFont(FONT , 24);
    if (!font_regular)
    {
        cerr << "Failed to load font_regular: " << TTF_GetError() << endl;
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

    Load_Scores();

    // box2d initializations
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 50.0f};
    World_Id = b2CreateWorld(&worldDef);

    // Window size
    SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Objects
    m_Player = make_unique<Player>(World_Id);
    Generate_Initial_Ground();

    // Sounds
    Audio_Manager::GetInstance().Init();
    Audio_Manager::GetInstance().LoadSound("jump", "D://SOUND_EFFECTS//qubodup-cfork-ccby3-jump.ogg");
    Audio_Manager::GetInstance().LoadSound("crash", "D://SOUND_EFFECTS//zoom3.wav");
    Audio_Manager::GetInstance().LoadSound("UI_click", "D://SOUND_EFFECTS//UI//switch9.ogg");
}

void Game::Run()
{
    running = true;
    m_current_State = STATE::MAIN_MENU;

    const int TARGET_FPS = 60;
    const float FRAME_DELAY = 1000.0f / TARGET_FPS;
    float timeStep = 1.0f / TARGET_FPS;
    Uint32 frameStart;

    while (running)
    {
        frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            switch (m_current_State)
            {
                case STATE::MAIN_MENU:
                    HandleEvents_MainMenu(event);
                    break;
                case STATE::PLAYING:
                    HandleEvents_Playing(event);
                    break;
                case STATE::GAME_OVER:
                    HandleEvents_GameOver(event);
                    break;
            }
        }

        switch (m_current_State)
        {
            case STATE::MAIN_MENU:
                Render_MainMenu();
                break;

            case STATE::PLAYING:
                Update_Playing(timeStep);
                Render_Playing();
                break;

            case STATE::GAME_OVER:
                Render_GameOver();
                break;
        }
        SDL_RenderPresent(renderer);

        int frameTime = SDL_GetTicks() - frameStart;
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
    // Decrease the spawn timer by the time elapsed this frame
    m_Obstacle_Spawn_Timer -= deltaTime;

    // If the timer has run out, it's time to spawn something
    if (m_Obstacle_Spawn_Timer <= 0.0f)
    {
        // --- 1. Spawn an Obstacle ---
        const float groundSurfaceY = SCREEN_HEIGHT - 40.0f; // 40px ground height
        float spawnX = cameraX + SCREEN_WIDTH + 100; // Spawn 100px off-screen to the right

        // Randomly choose which type of obstacle to create
        int obstacleType = rand() % 3; // Random number between 0 and 2

        switch (obstacleType)
        {
            case 0: // Short Obstacle (jump over)
            {
                float width = 50.0f;
                float height = 50.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
            case 1: // Tall Obstacle (requires double jump)
            {
                float width = 50.0f;
                float height = 100.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
            case 2: // Wide Obstacle (requires precise jump timing)
            {
                float width = 150.0f;
                float height = 25.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
                break;
            }
        }

        // --- 2. Potentially Spawn a Power-Up ---
        // Only try to spawn a power-up if an obstacle was just created.
        if (!m_Obstacles.empty() && rand() % 4 == 0) // 25% chance
        {
            Obstacle* lastObstacle = m_Obstacles.back().get();
            b2Vec2 obstaclePos = lastObstacle->get_position();
            float obstacleWidth = lastObstacle->GetWidthMeters();

            // Randomly choose a position relative to the obstacle
            int posType = rand() % 3;
            b2Vec2 powerUpPos;

            if (posType == 0) { // Before obstacle
                powerUpPos = { obstaclePos.x - obstacleWidth, obstaclePos.y - 1.5f };
            } else if (posType == 1) { // On top of obstacle
                powerUpPos = { obstaclePos.x, obstaclePos.y - 3.0f };
            } else { // After obstacle
                powerUpPos = { obstaclePos.x + obstacleWidth, obstaclePos.y - 1.5f };
            }

            // Randomly choose a power-up type
            PowerUpType type = static_cast<PowerUpType>(rand() % 3);
            m_powerUps.push_back(std::make_unique<PowerUp>(World_Id, type, powerUpPos.x * PIXELS_PER_METER, powerUpPos.y * PIXELS_PER_METER));
        }

        // --- 3. Reset the Timer ---
        // Set the timer for the *next* obstacle to a new random duration.
        float randomTime = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.5f));
        m_Obstacle_Spawn_Timer = randomTime;
    }
}

//void Game::Update_Spawning(float deltaTime)
//{
//    // Decrease the timer
//    m_Obstacle_Spawn_Timer -= deltaTime;
//
//    // If the timer has run out, it's time to spawn a new obstacle
//    if (m_Obstacle_Spawn_Timer <= 0.0f)
//    {
//        const float groundSurfaceY = SCREEN_HEIGHT - 40.0f; // 40px ground height
//        float spawnX = cameraX + SCREEN_WIDTH + 100; // Spawn 100px off-screen to the right
//
//        // Randomly choose an obstacle type to spawn
//        int obstacleType = rand() % 3;
//
//        switch (obstacleType)
//        {
//            case 0: // Short Obstacle (jump over)
//            {
//                float width = 50.0f;
//                float height = 50.0f;
//                float spawnY = groundSurfaceY - (height / 2.0f);
//                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
//                break;
//            }
//            case 1: // Tall Obstacle (requires double jump)
//            {
//                float width = 50.0f;
//                float height = 125.0f;
//                float spawnY = groundSurfaceY - (height / 2.0f);
//                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
//                break;
//            }
//            case 2: // Wide Obstacle (requires precise jump timing)
//            {
//                float width = 200.0f;
//                float height = 50.0f;
//                float spawnY = groundSurfaceY - (height / 2.0f);
//                m_Obstacles.push_back(make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height));
//                break;
//            }
//        }
//
//        float randomTime = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.5f));
//        m_Obstacle_Spawn_Timer = randomTime;
//    }
//
//    if (rand() % 4 == 0) // 25% chance to spawn a power-up
//    {
//        Obstacle* lastObstacle = m_Obstacles.back().get();
//        b2Vec2 obstaclePos = lastObstacle->get_position();
//        float obstacleWidth = lastObstacle->GetWidthMeters();
//
//        // Randomly choose a position relative to the obstacle
//        int posType = rand() % 3;
//        b2Vec2 powerUpPos;
//        if (posType == 0) { // Before obstacle
//            powerUpPos = { obstaclePos.x - obstacleWidth, obstaclePos.y - 1.5f };
//        } else if (posType == 1) { // On top of obstacle
//            powerUpPos = { obstaclePos.x, obstaclePos.y - 3.0f };
//        } else { // After obstacle
//            powerUpPos = { obstaclePos.x + obstacleWidth, obstaclePos.y - 1.5f };
//        }
//
//        // Randomly choose a power-up type
//        PowerUpType type = static_cast<PowerUpType>(rand() % 3);
//        m_powerUps.push_back(std::make_unique<PowerUp>(World_Id, type, powerUpPos.x * PIXELS_PER_METER, powerUpPos.y * PIXELS_PER_METER));
//    }
//}

void Game::Update_Score()
{
    // Get the player's current X position in meters
    float playerX = m_Player->get_position().x;

    // Loop through all obstacles
    for (const auto& obstacle : m_Obstacles)
    {
        // Check if the obstacle hasn't been scored yet
        if (!obstacle->Is_Scored())
        {
            float obstacleX = obstacle->get_position().x;

            // If the player has moved past the obstacle
            if (playerX > obstacleX)
            {
                int points = (m_Player->HasDoubleScore()) ? 2 : 1; // New method in Player
                m_score += points;
                obstacle->Set_Scored(true);
                cout << "Score: " << m_score << endl;
            }
        }
    }
}

void Game::Render_UI()
{
    string scoreText = "Score: " + to_string(m_score);
    SDL_Color textColor = { 50, 50, 50, 255 };

    render_text(renderer, font_large, scoreText, SCREEN_WIDTH / 2 - 75, 20, textColor);

    if (m_Player->HasExtraJump())
        render_text(renderer, font_regular, "Extra Jump: " + to_string(m_Player->get_extra_jump_timer()), 20, 20);
    if (m_Player->HasDoubleScore())
        render_text(renderer, font_regular, "Double Score: " + to_string(m_Player->get_double_score_timer()), 20, 20);
}

void Game::Reset_Game()
{
    m_score = 0;
    m_Player->SetIsDead(false);

    m_Player->Reset();

    m_Obstacles.clear();
    m_Ground_Segments.clear();

    Generate_Initial_Ground();
    cameraX = 0.0f;

    m_Obstacle_Spawn_Timer = 3.0f;
}

void Game::Render_Playing()
{
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

    for (const auto& powerUp : m_powerUps)
    {
        powerUp->Render(renderer, cameraX);
    }

    Render_UI();
}

void Game::Update_Playing(float timeStep)
{
    // --- 1. Update Game Objects ---
    m_Player->Update(World_Id, timeStep);

    Update_Ground();
    Update_Spawning(timeStep);
    Update_Score();

    // --- 2. Step the Physics World ---
    b2World_Step(World_Id, timeStep, 3);

    // --- 3. Check for Collisions ---
    if (!m_Player->IsDead())
    {
        b2Vec2 playerCenter = m_Player->get_position();
        float playerRadius = m_Player->Get_Radius_Meters();

        for (const auto& obstacle : m_Obstacles)
        {
            b2Vec2 obstacleCenter = obstacle->get_position();
            float obstacleHalfWidth = obstacle->GetWidthMeters() / 2.0f;
            float obstacleHalfHeight = obstacle->GetHeightMeters() / 2.0f;

            // Find the closest point on the obstacle's box to the player's center
            float closestX = std::max(obstacleCenter.x - obstacleHalfWidth, std::min(playerCenter.x, obstacleCenter.x + obstacleHalfWidth));
            float closestY = std::max(obstacleCenter.y - obstacleHalfHeight, std::min(playerCenter.y, obstacleCenter.y + obstacleHalfHeight));

            // Calculate squared distance from the player's center to this closest point
            float deltaX = playerCenter.x - closestX;
            float deltaY = playerCenter.y - closestY;
            float distanceSq = (deltaX * deltaX) + (deltaY * deltaY);

            // Compare that distance to the player's radius squared
            if (distanceSq < (playerRadius * playerRadius))
            {
                Audio_Manager::GetInstance().PlaySound("crash");
                m_Player->SetIsDead(true);
                break;
            }
        }
    }

    // --- NEW: Player vs Power-up Collision Check ---
    if (!m_Player->IsDead())
    {
        b2Vec2 playerCenter = m_Player->get_position();
        float playerRadius = m_Player->Get_Radius_Meters();

        // Use a safe iterator loop to handle removing collected power-ups
        for (auto it = m_powerUps.begin(); it != m_powerUps.end(); /* no increment */)
        {
            PowerUp* powerUp = it->get();
            b2Vec2 powerUpCenter = powerUp->get_position();
            float powerUpRadius = 20.0f / PIXELS_PER_METER; // Assuming 20px radius

            float distanceSq = b2DistanceSquared(playerCenter, powerUpCenter);

            if (distanceSq < (playerRadius + powerUpRadius) * (playerRadius + powerUpRadius))
            {
                // Collision! Activate the power-up and remove it.
                m_Player->ActivatePowerUp(powerUp->GetType());
                it = m_powerUps.erase(it); // Erase and get next valid iterator
            }
            else
            {
                ++it; // No collision, just move to the next item
            }
        }
    }

    // --- 4. Check for State Change ---
    if (m_Player->IsDead())
    {
        Audio_Manager::GetInstance().PlaySound("crash");
        Update_High_Scores();
        m_current_State = STATE::GAME_OVER;
    }

    // --- 5. Update the Camera ---
    b2Vec2 playerPosMeters = m_Player->get_position();
    float playerPosPixelsX = playerPosMeters.x * PIXELS_PER_METER;
    cameraX = playerPosPixelsX - (SCREEN_WIDTH / 2.0f);
}

void Game::Render_GameOver()
{
    // First, draw the last frame of the game
    Render_Playing();

    // Then, draw the game over UI on top
    render_text(renderer, font_large, "Game Over", SCREEN_WIDTH / 2 - 120, 200);

    string scoreText = "Final Score: " + to_string(m_score);
    render_text(renderer, font_regular, scoreText, SCREEN_WIDTH / 2 - 75, 320);

    // Draw buttons
    render_text(renderer, font_regular, "Restart", SCREEN_WIDTH / 2 - 45, 400);
    render_text(renderer, font_regular, "Main Menu", SCREEN_WIDTH / 2 - 65, 460);
}

void Game::Render_MainMenu()
{
//    // Clear the screen to a background color
//    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
//    SDL_RenderClear(renderer);
    Render_Playing();

    // --- Draw Title ---
    render_text(renderer, font_large, "Endless Runner", SCREEN_WIDTH / 2 - 150, 100);

    // --- Draw Buttons ---
    SDL_Rect startButtonRect = { SCREEN_WIDTH / 2 - 95, 300, 200, 50 };
    SDL_Rect quitButtonRect = { SCREEN_WIDTH / 2 - 95, 360, 200, 50 };

    // Draw button backgrounds
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &startButtonRect);
    SDL_RenderDrawRect(renderer, &quitButtonRect);

    // Draw button text
    render_text(renderer, font_regular, "Start", SCREEN_WIDTH / 2 - 25, 310);
    render_text(renderer, font_regular, "Quit", SCREEN_WIDTH / 2 - 20, 370);

    // --- Draw High Scores Box ---
    render_text(renderer, font_regular, "High Scores", 100, 250);

    int yPos = 300;
    for (size_t i = 0; i < m_high_Scores.size() && i < 5; ++i)
    {
        string scoreText = to_string(i + 1) + ". " + to_string(m_high_Scores[i]);
        render_text(renderer, font_regular, scoreText, 100, yPos);
        yPos += 40;
    }
}

void Game::HandleEvents_Playing(const SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_SPACE)
        {
            if (m_Player->Can_Jump())
            {
                m_Player->Jump();
            }
        }
    }
}

void Game::HandleEvents_MainMenu(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = { mouseX, mouseY };

        SDL_Rect startButtonRect = { SCREEN_WIDTH / 2 - 95, 300, 200, 50 };
        SDL_Rect quitButtonRect = { SCREEN_WIDTH / 2 - 95, 360, 200, 50 };

        if (SDL_PointInRect(&mousePoint, &startButtonRect))
        {
            Audio_Manager::GetInstance().PlaySound("UI_click");
            Reset_Game();
            m_current_State = STATE::PLAYING;
        }
        else if (SDL_PointInRect(&mousePoint, &quitButtonRect))
        {
            Audio_Manager::GetInstance().PlaySound("UI_click");
            running = false;
            Audio_Manager::GetInstance().CleanUp();
        }
    }
}

void Game::HandleEvents_GameOver(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = { mouseX, mouseY };

        int restartTextW, restartTextH;
        TTF_SizeText(font_regular, "Restart", &restartTextW, &restartTextH);
        SDL_Rect restartButtonRect = {
                SCREEN_WIDTH / 2 - restartTextW / 2,
                400,
                restartTextW,
                restartTextH
        };

        int menuTextW, menuTextH;
        TTF_SizeText(font_regular, "Main Menu", &menuTextW, &menuTextH);
        SDL_Rect menuButtonRect = {
                SCREEN_WIDTH / 2 - menuTextW / 2,
                460,
                menuTextW,
                menuTextH
        };

        if (SDL_PointInRect(&mousePoint, &restartButtonRect))
        {
            Audio_Manager::GetInstance().PlaySound("UI_click");
            Reset_Game();
            m_current_State = STATE::PLAYING;
        }
        else if (SDL_PointInRect(&mousePoint, &menuButtonRect))
        {
            Audio_Manager::GetInstance().PlaySound("UI_click");
            m_current_State = STATE::MAIN_MENU;
        }
    }
}

// Saving Scores

void Game::Save_Scores()
{
    ofstream scoreFile("scores.txt");
    if (scoreFile.is_open())
    {
        for (int score : m_high_Scores)
        {
            scoreFile << score << endl;
        }
        scoreFile.close();
    }
}

void Game::Load_Scores()
{
    ifstream scoreFile("scores.txt");
    if (scoreFile.is_open())
    {
        m_high_Scores.clear();
        string line;
        while (getline(scoreFile, line))
        {
            m_high_Scores.push_back(stoi(line));
        }
        scoreFile.close();
    }
}

void Game::Update_High_Scores()
{
    m_high_Scores.push_back(m_score);

    sort(m_high_Scores.begin(), m_high_Scores.end(), std::greater<int>());

    if (m_high_Scores.size() > 5)
    {
        m_high_Scores.resize(5);
    }

    Save_Scores();
}