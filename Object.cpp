//
// Created by amirh on 2025-08-12.
//

#include "Object.h"

// Player

Player::Player(b2WorldId worldId)
{
    is_Dead = false;
    const float PLAYER_RADIUS_PX = 25.0f;

    // Define the body's properties
    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_dynamicBody;
    body_def.position = { 400.0f / PIXELS_PER_METER, 400.0f / PIXELS_PER_METER };
    body_def.userData = this;

    // Create the body
    Body_Id = b2CreateBody(worldId, &body_def);

    b2Circle circle;
    circle.radius = PLAYER_RADIUS_PX / PIXELS_PER_METER;

    b2ShapeDef dynamic_shape_def = b2DefaultShapeDef();
    dynamic_shape_def.density = 1.0f;

    b2ShapeId circle_shape_ID = b2CreateCircleShape(Body_Id, &dynamic_shape_def, &circle);

    b2Shape_SetFriction(circle_shape_ID, 0.7f);
    b2Shape_SetRestitution(circle_shape_ID, 0.0f);
}

void Player::Jump()
{
    b2Vec2 impulse = { 0.0f, -40.0f };
    b2Body_ApplyLinearImpulseToCenter(Body_Id, impulse, true);
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

void Player::Update()
{
    b2Body_SetAngularVelocity(Body_Id, 0.0f);

    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

    if (keyboardState[SDL_SCANCODE_D])
    {
        Move_Right();
    }
    else if (keyboardState[SDL_SCANCODE_A])
    {
        Move_Left();
    }
}

void Player::Render(SDL_Renderer* renderer, float cameraX)
{
    const float PLAYER_WIDTH_PX = 50.0f;
    const float PLAYER_HEIGHT_PX = 50.0f;

    b2Vec2 player_pos = b2Body_GetPosition(Body_Id);
    SDL_Rect player_rect = {
            (int)((player_pos.x * PIXELS_PER_METER) - (PLAYER_WIDTH_PX / 2.0f) - cameraX),
            (int)((player_pos.y * PIXELS_PER_METER) - (PLAYER_HEIGHT_PX / 2.0f)),
            (int)PLAYER_WIDTH_PX,
            (int)PLAYER_HEIGHT_PX
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &player_rect);
}

// Scenery

Scenery::Scenery(b2WorldId worldId, float startX)
{
    const float Ground_Height_Px = 20.0f;
    m_Width_Meters = SCREEN_WIDTH / PIXELS_PER_METER;

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.type = b2_staticBody;

    groundBodyDef.position = {
            startX / PIXELS_PER_METER + m_Width_Meters / 2.0f,
            (SCREEN_HEIGHT - (Ground_Height_Px / 2.0f)) / PIXELS_PER_METER
    };

    groundBodyDef.userData = this;

    Body_Id = b2CreateBody(worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(m_Width_Meters / 2.0f, (Ground_Height_Px / 2.0f) / PIXELS_PER_METER);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();

    b2ShapeId ground_shape_ID = b2CreatePolygonShape(Body_Id, &groundShapeDef, &groundBox);

    b2Shape_SetRestitution(ground_shape_ID, 0.0f);
    b2Shape_SetFriction(ground_shape_ID, 0.7f);
}

void Scenery::Update()
{

}

void Scenery::Render(SDL_Renderer* renderer, float cameraX)
{
    const float GROUND_WIDTH_PX = SCREEN_WIDTH;
    const float GROUND_HEIGHT_PX = 20.0f;

    b2Vec2 scenery_pos = b2Body_GetPosition(Body_Id);
    SDL_Rect scenery_rect = {
            (int)((scenery_pos.x * PIXELS_PER_METER) - (GROUND_WIDTH_PX / 2.0f) - cameraX),
            (int)((scenery_pos.y * PIXELS_PER_METER) - (GROUND_HEIGHT_PX / 2.0f)),
            (int)GROUND_WIDTH_PX,
            (int)GROUND_HEIGHT_PX
    };

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(renderer, &scenery_rect);
}

Scenery::~Scenery()
{
    if (b2Body_IsValid(Body_Id))
    {
        b2DestroyBody(Body_Id);
    }
}

float Scenery::Get_Right_EdgeX() const
{
    b2Vec2 pos = b2Body_GetPosition(Body_Id);
    return (pos.x + m_Width_Meters / 2.0f) * PIXELS_PER_METER;
}

// Obstacles
Obstacle::Obstacle(b2WorldId worldId, float x, float y)
{
    // Obstacle size
    m_Width_Px = 50.0f;
    m_Height_Px = 50.0f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;

    // Set its position based on the arguments
    bodyDef.position = { x / PIXELS_PER_METER, y / PIXELS_PER_METER };

    // Tag the body with a pointer to this object
    bodyDef.userData = this;

    Body_Id = b2CreateBody(worldId, &bodyDef);

    // hitbox shape
    b2Polygon box = b2MakeBox(
            (m_Width_Px / 2.0f) / PIXELS_PER_METER,
            (m_Height_Px / 2.0f) / PIXELS_PER_METER
    );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(Body_Id, &shapeDef, &box);
}

void Obstacle::Render(SDL_Renderer* renderer, float cameraX)
{
    const float visual_Y_Offset = 20.0f;

    b2Vec2 position = b2Body_GetPosition(Body_Id);
    SDL_Rect rect = {
            (int)((position.x * PIXELS_PER_METER) - (m_Width_Px / 2.0f) - cameraX),
            (int)((position.y * PIXELS_PER_METER) - (m_Height_Px / 2.0f) + visual_Y_Offset),
            (int)m_Width_Px,
            (int)m_Height_Px
    };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

Obstacle::~Obstacle() noexcept
{

}

void Obstacle::Update()
{

}

float Obstacle::Get_Right_EdgeX() const
{
    b2Vec2 pos = b2Body_GetPosition(Body_Id);
    return (pos.x * PIXELS_PER_METER) + (m_Width_Px / 2.0f);
}