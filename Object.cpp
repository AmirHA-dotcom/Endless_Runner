//
// Created by amirh on 2025-08-12.
//

#include "Object.h"

// helper

#include "Object.h" // Make sure this is included

inline float RaycastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
{
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    // Get the game object pointer from the body's user data
    Object* hitObject = static_cast<Object*>(b2Body_GetUserData(bodyId));

//    if (hitObject != nullptr)
//    {
//        // Use dynamic_cast to check the actual type of the object
//        if (dynamic_cast<Player*>(hitObject))
//        {
//            std::cout << "Raycast Hit: Player" << std::endl;
//        }
//        else if (dynamic_cast<Scenery*>(hitObject))
//        {
//            std::cout << "Raycast Hit: Scenery" << std::endl;
//        }
//        else if (dynamic_cast<Obstacle*>(hitObject))
//        {
//            std::cout << "Raycast Hit: Obstacle" << std::endl;
//        }
//        else
//        {
//            std::cout << "Raycast Hit: Unknown Object" << std::endl;
//        }
//    }

    bool* hit = (bool*)context;
    *hit = true;

    return 0.0f; // Terminate the raycast
}

Object::~Object()
{
    if (B2_IS_NON_NULL(Body_Id))
    {
        b2DestroyBody(Body_Id);
    }
}

// Player

Player::Player(b2WorldId worldId)
{
    m_currentSkin = "player";
    m_texture = Asset_Manager::GetInstance().GetTexture(m_currentSkin);
    if (m_texture == nullptr) return;

    // Set animation properties
    m_frameCount = 2; // The Kenney sprite sheet you showed has 11 frames of walking
    m_currentFrame = 0;
    m_animTimer = 0.0f;
    m_animSpeed = 0.08f; // Display each frame for 0.08 seconds (~12 FPS animation)

    if (m_texture)
    {
        // Query the texture to get its total width and height
        int textureWidth, textureHeight;
        SDL_QueryTexture(m_texture, NULL, NULL, &textureWidth, &textureHeight);

        // Calculate the width of a single frame
//        m_frameWidth = textureWidth / m_frameCount;
//        m_frameHeight = textureHeight; // Since all frames are in one row
        m_frameWidth = 69;
        m_frameHeight = 94;
    }

    is_Dead = false;
    m_jumps_Left = 2;
    const float PLAYER_RADIUS_PX = 25.0f;

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_dynamicBody;
    body_def.position = { 300.0f / PIXELS_PER_METER, 700.0f / PIXELS_PER_METER };
    body_def.userData = this;

    Body_Id = b2CreateBody(worldId, &body_def);

    b2Circle circle;
    circle.radius = PLAYER_RADIUS_PX / PIXELS_PER_METER;

    b2Filter filter;
    filter.categoryBits = PLAYER_CATEGORY;
    filter.maskBits = GROUND_CATEGORY | OBSTACLE_CATEGORY;

    b2ShapeDef dynamic_shape_def = b2DefaultShapeDef();
    dynamic_shape_def.density = 1.0f;
    dynamic_shape_def.filter = filter;

    b2ShapeId circle_shape_ID = b2CreateCircleShape(Body_Id, &dynamic_shape_def, &circle);

    b2Shape_SetFriction(circle_shape_ID, 0.7f);
    b2Shape_SetRestitution(circle_shape_ID, 0.0f);
}

bool Player::Can_Jump() const
{
    return m_jumps_Left > 0;
}

void Player::Jump()
{
    if (!Can_Jump())
    {
        return;
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(Body_Id);
    velocity.y = -20.0f;
    b2Body_SetLinearVelocity(Body_Id, velocity);

    Audio_Manager::GetInstance().PlaySound("jump");
    m_jumps_Left--;
}

void Player::Move_Right()
{
    b2Vec2 current_velocity = b2Body_GetLinearVelocity(Body_Id);

    current_velocity.x = PLAYER_SPEED;

    b2Body_SetLinearVelocity(Body_Id, current_velocity);
}

void Player::Move_Left()
{
    b2Vec2 current_velocity = b2Body_GetLinearVelocity(Body_Id);

    current_velocity.x = -PLAYER_SPEED;

    b2Body_SetLinearVelocity(Body_Id, current_velocity);
}

bool Player::Is_On_Ground(b2WorldId worldId)
{
    b2Vec2 position = get_position();
    float playerRadius = Get_Radius_Meters();

    b2Vec2 startPoint = position;
    b2Vec2 endPoint = {position.x, position.y + playerRadius + 0.1f};

    b2QueryFilter filter = b2DefaultQueryFilter();

    filter.maskBits = GROUND_CATEGORY | OBSTACLE_CATEGORY;

    bool hit = false;
    void* context = &hit;

    b2World_CastRay(worldId, startPoint, endPoint, filter, RaycastCallback, context);

    return hit;
}

void Player::Update(b2WorldId worldId, float deltaTime, int score)
{
    PLAYER_SPEED = BASE_SPEED + (score / 5) * 2;

    // Clamp the speed to the maximum value
    if (PLAYER_SPEED > MAX_SPEED)
    {
        PLAYER_SPEED = MAX_SPEED;
    }

    b2Body_SetAngularVelocity(Body_Id, 0.0f);
    Move_Right();

    // Check if the player is on the ground
    if (Is_On_Ground(worldId))
    {
        SetAnimation(AnimationState::RUNNING);

        b2Vec2 velocity = b2Body_GetLinearVelocity(Body_Id);

        if (velocity.y >= 0.0f)
        {
            if (m_extraJumpTimer > 0.0f)
            {
                m_jumps_Left = 3;
            }
            else
            {
                m_jumps_Left = MAX_JUMPS;
            }
        }
    }
    else // The player is in the air
    {
        b2Vec2 velocity = b2Body_GetLinearVelocity(Body_Id);
        std::cout << "In Air, Y-Velocity: " << velocity.y << std::endl;

        if (velocity.y < -0.1f) // Moving upwards (jumping)
        {
            SetAnimation(AnimationState::JUMPING);
        }
        else if (velocity.y > 0.1f) // Moving downwards (falling)
        {
            SetAnimation(AnimationState::FALLING);
        }
    }

    if (m_animState == AnimationState::RUNNING)
    {
        m_animTimer += deltaTime;
        if (m_animTimer >= m_animSpeed)
        {
            // Move to the next frame
            m_currentFrame++;

            // If we've passed the last frame, loop back to the first
            if (m_currentFrame >= m_frameCount)
            {
                m_currentFrame = 0;
            }

            // Reset the timer for the next frame
            m_animTimer -= m_animSpeed;
        }
    }

    if (m_extraJumpTimer > 0)
    {
        m_extraJumpTimer -= deltaTime;
    }

    if (m_doubleScoreTimer > 0)
    {
        m_doubleScoreTimer -= deltaTime;
    }
}

void Player::Render(SDL_Renderer* renderer, float cameraX)
{
    if (m_texture == nullptr) return;

    SDL_Rect srcRect;
    srcRect.x = m_currentFrame * m_frameWidth; // Calculate X position on the sprite sheet
//    srcRect.y = 0;                             // Top of the sheet
    srcRect.y = m_animRow * m_frameHeight + 4;
    srcRect.w = m_frameWidth;                  // Width of a single frame
    srcRect.h = m_frameHeight;                 // Height of a single frame


    const float PLAYER_WIDTH_PX = 50.0f;
    const float PLAYER_HEIGHT_PX = 50.0f;

    b2Vec2 player_pos = b2Body_GetPosition(Body_Id);
    SDL_Rect player_rect = {
            (int)((player_pos.x * PIXELS_PER_METER) - (PLAYER_WIDTH_PX / 2.0f) - cameraX),
            (int)((player_pos.y * PIXELS_PER_METER) - (PLAYER_HEIGHT_PX / 2.0f)),
            (int)PLAYER_WIDTH_PX,
            (int)PLAYER_HEIGHT_PX
    };

    //SDL_RenderCopy(renderer, m_texture, NULL, &player_rect);
    SDL_RenderCopy(renderer, m_texture, &srcRect, &player_rect);


//    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
//    SDL_RenderFillRect(renderer, &player_rect);
}

float Player::Get_Radius_Meters() const
{
    const float actual_Radius = 25.0f / PIXELS_PER_METER;
    const float forgiveness_Factor = 0.85f;
    return actual_Radius * forgiveness_Factor;
}

void Player::Reset()
{
    b2Vec2 startPosition = { 300.0f / PIXELS_PER_METER, 700.0f / PIXELS_PER_METER };
    //b2Body_SetTransform(Body_Id, startPosition, 0.0f); // Set position and angle
    b2Body_SetLinearVelocity(Body_Id, b2Vec2_zero);    // Stop all movement
    b2Body_SetAngularVelocity(Body_Id, 0.0f);          // Stop all rotation

    m_extraJumpTimer = 0.0f;
    m_doubleScoreTimer = 0.0f;

    m_jumps_Left = MAX_JUMPS;
}

void Player::ActivatePowerUp(PowerUpType type)
{
    switch (type)
    {
        case PowerUpType::EXTRA_JUMP:
            m_extraJumpTimer = 10.0f;
            break;
        case PowerUpType::DOUBLE_SCORE:
            m_doubleScoreTimer = 10.0f;
            break;
    }
}

void Player::SetAnimation(AnimationState state)
{
    // Don't restart the animation if we are already in the same state
    if (state == m_animState) return;

    m_animState = state;

    switch (m_animState)
    {
        case AnimationState::RUNNING:
            m_animRow = 2;      // 3rd row
            m_frameCount = 2;   // 2 frames
            m_animSpeed = 0.2f;
            break;
        case AnimationState::JUMPING:
            m_animRow = 3;      // 4th row
            m_currentFrame = 1; // 2nd frame on that row
            m_frameCount = 1;   // It's a single static frame
            cout << "JUMPED" << endl;
            break;
        case AnimationState::FALLING:
            m_animRow = 3;      // 4th row
            m_currentFrame = 0; // 1st frame on that row
            m_frameCount = 1;   // Also a single static frame
            break;
    }
}

// Scenery

Scenery::Scenery(b2WorldId worldId, float startX, SDL_Texture* texture)
{
    m_texture = texture;

    const float Ground_Height_Px = 20.0f;
    m_Width_Meters = SCREEN_WIDTH / PIXELS_PER_METER;

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.type = b2_staticBody;

    groundBodyDef.position = {
            startX / PIXELS_PER_METER + m_Width_Meters / 2.0f,
            (SCREEN_HEIGHT - (Ground_Height_Px / 2.0f)) / PIXELS_PER_METER
    };

    groundBodyDef.userData = this;

    //std::cout << "Creating Scenery at: " << groundBodyDef.position.x << ", " << groundBodyDef.position.y << std::endl;
    Body_Id = b2CreateBody(worldId, &groundBodyDef);

    b2Filter filter;
    filter.categoryBits = GROUND_CATEGORY;
    filter.maskBits = PLAYER_CATEGORY;

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.filter = filter;

    b2Polygon groundBox = b2MakeBox(m_Width_Meters / 2.0f, (Ground_Height_Px / 2.0f) / PIXELS_PER_METER);
    b2ShapeId ground_shape_ID = b2CreatePolygonShape(Body_Id, &groundShapeDef, &groundBox);

    b2Shape_SetRestitution(ground_shape_ID, 0.0f);
    b2Shape_SetFriction(ground_shape_ID, 0.7f);
}

void Scenery::Update(b2WorldId worldId, float deltaTime, int score)
{

}

void Scenery::Render(SDL_Renderer* renderer, float cameraX)
{
    if (m_texture == nullptr) return;

    // Get the dimensions of a single tile from the texture
    int tileWidth, tileHeight;
    SDL_QueryTexture(m_texture, NULL, NULL, &tileWidth, &tileHeight);

    // Get the position and width of the entire ground segment in pixels
    b2Vec2 segmentPos = b2Body_GetPosition(Body_Id);
    float segmentWidthPx = m_Width_Meters * PIXELS_PER_METER;
    float segmentTopLeftX = (segmentPos.x * PIXELS_PER_METER) - (segmentWidthPx / 2.0f);
    float segmentTopLeftY = (segmentPos.y * PIXELS_PER_METER) - (tileHeight / 2.0f); // Assuming render height = tile height

    // Calculate how many tiles are needed to cover the segment's width
    int numTiles = static_cast<int>(ceil(segmentWidthPx / tileWidth));

    // Loop and draw the tile repeatedly
    for (int i = 0; i < numTiles; ++i)
    {
        SDL_Rect destRect = {
                static_cast<int>(segmentTopLeftX + (i * tileWidth) - cameraX),
                static_cast<int>(segmentTopLeftY),
                tileWidth,
                tileHeight
        };

        SDL_RenderCopy(renderer, m_texture, NULL, &destRect);
    }
}

Scenery::~Scenery()
{
//    if (b2Body_IsValid(Body_Id))
//    {
//        b2DestroyBody(Body_Id);
//    }
}

float Scenery::Get_Right_EdgeX() const
{
    b2Vec2 pos = b2Body_GetPosition(Body_Id);
    return (pos.x + m_Width_Meters / 2.0f) * PIXELS_PER_METER;
}

// Obstacles

Obstacle::Obstacle(b2WorldId worldId, float x, float y, float width, float height, SDL_Texture* texture)
{
    m_texture = texture;
    // Store the obstacle's size in pixels
    m_Width_Px = width;
    m_Height_Px = height;

    // --- 1. Define the Body ---
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody; // Obstacles don't move
    bodyDef.position = { x / PIXELS_PER_METER, y / PIXELS_PER_METER };
    bodyDef.userData = this; // Tag the body with a pointer to this object

    // Create the body in the world
    //std::cout << "Creating Obstacle at: " << bodyDef.position.x << ", " << bodyDef.position.y << std::endl;

    Body_Id = b2CreateBody(worldId, &bodyDef);

    // --- 2. Define the Shape and its Properties ---
    b2Polygon box = b2MakeBox(
            (m_Width_Px / 2.0f) / PIXELS_PER_METER,
            (m_Height_Px / 2.0f) / PIXELS_PER_METER
    );

    // Define the collision filter for the obstacle
    b2Filter filter;
    filter.categoryBits = OBSTACLE_CATEGORY;
    // Obstacles only need to physically collide with the player
    filter.maskBits = PLAYER_CATEGORY;

    // Define the shape's properties using the filter
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter = filter;      // Apply the collision filter
    shapeDef.isSensor = true;     // This makes the obstacle Sensor

    // --- 3. Create the Shape in the World ---
    // Create the shape and get its ID
    b2ShapeId shape_ID = b2CreatePolygonShape(Body_Id, &shapeDef, &box);

    // Set the final material properties
    b2Shape_SetRestitution(shape_ID, 0.0f); // No bounce
    b2Shape_SetFriction(shape_ID, 0.5f);
}

void Obstacle::Render(SDL_Renderer* renderer, float cameraX)
{
    if (m_texture == nullptr) return;

    const float visual_Y_Offset = 20.0f;

    b2Vec2 position = b2Body_GetPosition(Body_Id);
    SDL_Rect rect = {
            (int)((position.x * PIXELS_PER_METER) - (m_Width_Px / 2.0f) - cameraX),
            (int)((position.y * PIXELS_PER_METER) - (m_Height_Px / 2.0f)),
            (int)m_Width_Px,
            (int)(m_Height_Px + visual_Y_Offset)
    };

//    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderCopy(renderer, m_texture, NULL, &rect);

}

Obstacle::~Obstacle() noexcept
{

}

void Obstacle::Update(b2WorldId worldId, float deltaTime, int score)
{

}

float Obstacle::Get_Right_EdgeX() const
{
    b2Vec2 pos = b2Body_GetPosition(Body_Id);
    return (pos.x * PIXELS_PER_METER) + (m_Width_Px / 2.0f);
}

// Power Ups

PowerUp::PowerUp(b2WorldId worldId, PowerUpType type, float x, float y) : m_type(type)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = { x, y };
    bodyDef.userData = this;

    //std::cout << "Creating PowerUp at: " << bodyDef.position.x << ", " << bodyDef.position.y << std::endl;

    Body_Id = b2CreateBody(worldId, &bodyDef);

    // All power-ups will be sensors
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true;

    // ... set filter data if needed ...

    b2Circle circle;
    circle.radius = 20.0f / PIXELS_PER_METER; // All power-ups are 20px radius circles
    b2CreateCircleShape(Body_Id, &shapeDef, &circle);

    switch (m_type)
    {
        case PowerUpType::EXTRA_JUMP:
            m_texture = Asset_Manager::GetInstance().GetTexture("powerUp");
            break;
        case PowerUpType::DOUBLE_SCORE:
            m_texture = Asset_Manager::GetInstance().GetTexture("powerUp");
            break;
    }

    m_frameCount = 10; // Both sprite sheets have 10 main animation frames
    m_animSpeed = 0.1f; // Adjust for desired rotation speed
    m_currentFrame = 0;
    m_animTimer = 0.0f;

    if (m_texture)
    {
        int textureWidth, textureHeight;
        SDL_QueryTexture(m_texture, NULL, NULL, &textureWidth, &textureHeight);
        m_frameWidth = textureWidth / 10;
        m_frameHeight = textureHeight;
    }
}

void PowerUp::Render(SDL_Renderer* renderer, float cameraX)
{
    if (m_texture == nullptr) return;

    // Source rect on the sprite sheet
    SDL_Rect srcRect = { m_currentFrame * m_frameWidth, 0, m_frameWidth, m_frameHeight };

    // Destination rect on the screen
    b2Vec2 position = b2Body_GetPosition(Body_Id);
    int radius = 20; // Visual radius in pixels
    SDL_Rect destRect = {
            static_cast<int>((position.x * PIXELS_PER_METER) - radius - cameraX),
            static_cast<int>((position.y * PIXELS_PER_METER) - radius),
            radius * 2,
            radius * 2
    };

    SDL_RenderCopy(renderer, m_texture, &srcRect, &destRect);
}

void PowerUp::Update(b2WorldId worldId, float deltaTime, int score)
{
    // This animation timer logic is the same as the Player's
    m_animTimer += deltaTime;
    if (m_animTimer >= m_animSpeed)
    {
        m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        m_animTimer -= m_animSpeed;
    }
}