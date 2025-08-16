//
// Created by amirh on 2025-08-09.
//

#include "Game.h"

// helper

inline void render_text(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y, SDL_Color color = { 200, 200, 200, 255 })
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

    // Textures
    Load_Assets();

    // BackGround
    GenerateStars();

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
    SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture("Ground_Sand");

    for (int i = 0; i < 3; ++i)
    {
        m_Ground_Segments.push_back(make_unique<Scenery>(World_Id, currentX, tex));
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
        SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture("Ground_Sand");
        float nextX = lastSegment->Get_Right_EdgeX();
        m_Ground_Segments.push_back(make_unique<Scenery>(World_Id, nextX, tex));
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
    // Decrease the spawn time
    m_Obstacle_Spawn_Timer -= deltaTime;

    if (m_Obstacle_Spawn_Timer <= 0.0f)
    {
        // Spawn an Obstacle
        const float groundSurfaceY = SCREEN_HEIGHT - 40.0f; // 40px ground height
        float spawnX = cameraX + SCREEN_WIDTH + 100; // Spawn 100px off-screen to the right

        // Randomly choose which type of obstacle to create
        int obstacleType = rand() % 3;

        switch (obstacleType)
        {
            case 0: // Short Obstacle (jump over)
            {
                float width = 70.0f;
                float height = 50.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
//                SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture("obstacle_small");
                const auto& skins = small_obstacle_skins;
                if (skins.empty()) break;

                int skinIndex = rand() % skins.size();
                SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture(skins[skinIndex]);

                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height, tex));
                break;
            }
            case 1: // Tall Obstacle (requires double jump)
            {
                float width = 70.0f;
                float height = 150.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                const auto& skins = tall_obstacle_skins;
                if (skins.empty()) break;

                int skinIndex = rand() % skins.size();
                SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture(skins[skinIndex]);
                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height, tex));
                break;
            }
            case 2: // Wide Obstacle (requires precise jump timing)
            {
                float width = 250.0f;
                float height = 25.0f;
                float spawnY = groundSurfaceY - (height / 2.0f);
                const auto& skins = wide_obstacle_skins;
                if (skins.empty()) break;

                int skinIndex = rand() % skins.size();
                SDL_Texture* tex = Asset_Manager::GetInstance().GetTexture(skins[skinIndex]);
                m_Obstacles.push_back(std::make_unique<Obstacle>(World_Id, spawnX, spawnY, width, height, tex));
                break;
            }
        }

        // Power Ups
        if (!m_Obstacles.empty() && rand() % 4 == 0)
        {
            Obstacle* lastObstacle = m_Obstacles.back().get();
            b2Vec2 obstaclePos = lastObstacle->get_position(); // meters
            float obstacleHalfWidth = lastObstacle->GetWidthMeters() / 2.0f; // meters
            float obstacleHalfHeight = lastObstacle->GetHeightMeters() / 2.0f; // meters

            // Power-up physical radius (meters) — keep consistent with PowerUp ctor
            const float powerUpRadius = 20.0f / PIXELS_PER_METER;
            const float safeMargin = 0.05f; // small margin in meters

            int posType = rand() % 3;
            b2Vec2 powerUpPos;

            // Compute using obstacle top surface and width (assuming obstaclePos.y is center and
            // smaller y = up / larger y = down as you already used obstaclePos.y - ... in code)
            float obstacleTopY = obstaclePos.y - obstacleHalfHeight;
            float obstacleCenterY = obstaclePos.y;

            if (posType == 0) { // Before obstacle (left)
                powerUpPos.x = obstaclePos.x - obstacleHalfWidth - powerUpRadius - safeMargin;
                // align vertically roughly to obstacle top area (tweak if needed)
                powerUpPos.y = obstacleTopY - (powerUpRadius * 0.1f);
            }
            else if (posType == 1) { // On top of obstacle
                powerUpPos.x = obstaclePos.x;
                powerUpPos.y = obstacleTopY - powerUpRadius - safeMargin;
            }
            else { // After obstacle (right)
                powerUpPos.x = obstaclePos.x + obstacleHalfWidth + powerUpRadius + safeMargin;
                powerUpPos.y = obstacleTopY - (powerUpRadius * 0.1f);
            }

            // Basic validation: no NaNs/infs and within reasonable world bounds
            if (!std::isfinite(powerUpPos.x) || !std::isfinite(powerUpPos.y)) {
                std::cout << "Skipping PowerUp spawn: invalid coordinate (NaN/inf)." << std::endl;
            } else {
                // Optionally clamp so power-up never appears below ground.
                // If ground Y (in meters) is needed, compute similarly to how obstacles were created:
                float groundY_m = (SCREEN_HEIGHT - 40.0f) / PIXELS_PER_METER; // if used elsewhere similarly
                // Ensure powerUp sits above ground surface
                float minAllowedY = powerUpRadius + 0.01f;
                float maxAllowedY = groundY_m - powerUpRadius - 0.01f;
                if (powerUpPos.y > maxAllowedY) powerUpPos.y = maxAllowedY;
                if (powerUpPos.y < minAllowedY) powerUpPos.y = minAllowedY;

                // Final defensive check
                if (!std::isfinite(powerUpPos.x) || !std::isfinite(powerUpPos.y)) {
                    std::cout << "Skipping PowerUp spawn: invalid after clamping." << std::endl;
                } else {
                    PowerUpType type = static_cast<PowerUpType>(rand() % 2);
                    std::cout << "Spawning PowerUp (meters): " << powerUpPos.x << ", " << powerUpPos.y << std::endl;
                    m_powerUps.push_back(std::make_unique<PowerUp>(World_Id, type, powerUpPos.x, powerUpPos.y));
                }
            }
        }

        //  Timer for Next Obstacle
        float baseMinDelay = 1.5f;
        float baseMaxDelay = 3.0f;
        float difficultyReduction = (m_score / 10) * 0.1f;
        float currentMinDelay = baseMinDelay - difficultyReduction;
        float currentMaxDelay = baseMaxDelay - difficultyReduction;

        const float absoluteMinDelay = 0.8f;
        const float absoluteMaxDelay = 1.5f;
        if (currentMinDelay < absoluteMinDelay) currentMinDelay = absoluteMinDelay;
        if (currentMaxDelay < absoluteMaxDelay) currentMaxDelay = absoluteMaxDelay;
        if (currentMaxDelay < currentMinDelay) currentMaxDelay = currentMinDelay;

        // Use a safer, more standard formula for random float generation.
        float delayRange = currentMaxDelay - currentMinDelay;
        float randomFraction = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float randomTime = currentMinDelay + (randomFraction * delayRange);

        m_Obstacle_Spawn_Timer = randomTime;
    }
}

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
    SDL_Color textColor = { 200, 200, 200, 255 };

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
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);

    RenderStarfield();

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

    m_Player->Render(renderer, cameraX);

    Render_UI();
}

void Game::Update_Playing(float timeStep)
{
    // Update Game Objects
    m_Player->Update(World_Id, timeStep, m_score);

    Update_Ground();
    Update_Spawning(timeStep);
    Update_Score();

    for (const auto& powerUp : m_powerUps)
    {
        powerUp->Update(World_Id, timeStep, m_score);
    }

    // Step the Physics World
    b2World_Step(World_Id, timeStep, 3);

    // Check for Collisions
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

            const float collisionBuffer = 0.1f; // A small buffer in meters
            float effectiveRadius = playerRadius + collisionBuffer;
            // Compare that distance to the player's radius squared
            if (distanceSq < (effectiveRadius * effectiveRadius))
            {
                m_Player->SetIsDead(true);
                break;
            }
        }
    }

    // Player vs Power-up Collision Check
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

    // Check for State Change
    if (m_Player->IsDead())
    {
        Audio_Manager::GetInstance().PlaySound("crash");
        Update_High_Scores();
        m_current_State = STATE::GAME_OVER;
    }

    // Update the Camera
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

// Textures

void Game::Load_Assets()
{
    // Player
//    Asset_Manager::GetInstance().LoadTexture("player", "D://Textures//kenney_platformer-art-deluxe//Base pack//Player//p1_stand.png", renderer);
    Asset_Manager::GetInstance().LoadTexture("player", "D://Textures//kenney_platformer-art-deluxe//Extra animations and enemies//Spritesheets//alienGreen.png", renderer);

    // Small Obstacles
    Asset_Manager::GetInstance().LoadTexture("obstacle_small_GreenMonster_Angry", "D://Textures//kenney_platformer-art-deluxe//Base pack//Enemies//blockerMad.png", renderer);
    Asset_Manager::GetInstance().LoadTexture("obstacle_small_Creature", "D://Textures//kenney_platformer-art-deluxe//Extra animations and enemies//Enemy sprites//barnacle.png", renderer);
    Asset_Manager::GetInstance().LoadTexture("obstacle_small_GreenMonster_Poker","D://Textures//kenney_platformer-art-deluxe//Extra animations and enemies//Enemy sprites//slimeBlock.png", renderer);
    small_obstacle_skins = { "obstacle_small_GreenMonster_Angry", "obstacle_small_Creature", "obstacle_small_GreenMonster_Poker" };

    // Tall Obstacles
    Asset_Manager::GetInstance().LoadTexture("obstacle_tall_OrangeMonster_Sad", "D://Textures//kenney_platformer-art-deluxe//Base pack//Enemies//pokerSad.png", renderer);
    Asset_Manager::GetInstance().LoadTexture("obstacle_tall_GreenMonster_Poker", "D://Textures//kenney_platformer-art-deluxe//Extra animations and enemies//Enemy sprites//snakeSlime.png", renderer);
    Asset_Manager::GetInstance().LoadTexture("obstacle_tall_OrangeMonster_Angry", "D://Textures//kenney_platformer-art-deluxe//Base pack//Enemies//pokerMad.png", renderer);
    tall_obstacle_skins = { "obstacle_tall_OrangeMonster_Sad", "obstacle_tall_GreenMonster_Poker", "obstacle_tall_OrangeMonster_Angry" };

    // Wide Obstacles
    Asset_Manager::GetInstance().LoadTexture("obstacle_wide", "D://Textures//kenney_platformer-art-deluxe//Extra animations and enemies//Enemy sprites//worm.png", renderer);
    wide_obstacle_skins = { "obstacle_wide" };

    // Power Ups
    Asset_Manager::GetInstance().LoadTexture("powerUp", "D://Textures//Game asset - Shining items sprite sheets v2//spritesheet _powerUp.png", renderer);

    // Coins
    Asset_Manager::GetInstance().LoadTexture("Coin", "D://Textures//Game asset - Shining items sprite sheets v2//Transparent PNG//Coin//frame-1.png", renderer);

    // Ground
    Asset_Manager::GetInstance().LoadTexture("Ground_Sand", "D://Textures//kenney_platformer-art-deluxe//Base pack//Tiles//sandCenter.png", renderer);


//    Asset_Manager::GetInstance().LoadTexture("obstacle_rock", "assets/images/kenney_foliage-pack/rock.png", renderer);
}

// BackGround

void Game::GenerateStars()
{
    const int NUM_LAYERS = 3;
    const int STARS_PER_LAYER[] = {50, 100, 150}; // Far, middle, near layers

    m_starLayers.resize(NUM_LAYERS);

    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        for (int j = 0; j < STARS_PER_LAYER[i]; ++j)
        {
            // Create stars across two screen widths to help with wrapping
            int x = rand() % (SCREEN_WIDTH * 2);
            int y = rand() % SCREEN_HEIGHT;
            m_starLayers[i].push_back({x, y});
        }
    }
}

void Game::RenderStarfield()
{
    const float LAYER_SPEEDS[] = {0.15f, 0.25f, 0.50f};

    const SDL_Color LAYER_COLORS[] = {
            {100, 100, 100, 255}, // Far stars
            {170, 170, 170, 255}, // Middle stars
            {255, 255, 255, 255}  // Near stars
    };

    for (int i = 0; i < m_starLayers.size(); ++i)
    {
        SDL_SetRenderDrawColor(renderer, LAYER_COLORS[i].r, LAYER_COLORS[i].g, LAYER_COLORS[i].b, LAYER_COLORS[i].a);

        // Calculate how much this layer should scroll based on the camera
        int parallaxX = static_cast<int>(cameraX * LAYER_SPEEDS[i]);

        for (const auto& star : m_starLayers[i])
        {
            int screenX = star.x - parallaxX;
            int screenY = star.y;

            // If the star scrolls off the left, wrap it to the right
            while (screenX < 0)
            {
                screenX += SCREEN_WIDTH * 2;
            }
            // Use modulo to keep the star within two screen widths
            screenX %= (SCREEN_WIDTH * 2);

            SDL_RenderDrawPoint(renderer, screenX, screenY);
        }
    }
}