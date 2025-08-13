//
// Created by amirh on 2025-08-12.
//

#include "Object.h"

// Player

Player::Player(b2WorldId worldId)
{
    const float PLAYER_HALF_WIDTH_PX = 25.0f;
    const float PLAYER_HALF_HEIGHT_PX = 25.0f;

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_dynamicBody;
    body_def.position = { 400 / PIXELS_PER_METER , 400 / PIXELS_PER_METER };
    Body_Id = b2CreateBody(worldId, &body_def);

    b2Polygon dynamic_box = b2MakeBox(PLAYER_HALF_WIDTH_PX / PIXELS_PER_METER, PLAYER_HALF_HEIGHT_PX / PIXELS_PER_METER);
    b2ShapeDef dynamic_shape_def = b2DefaultShapeDef();
    dynamic_shape_def.density = 1.0f;

    b2ShapeId box_shape_ID = b2CreatePolygonShape(Body_Id, &dynamic_shape_def, &dynamic_box);
    b2Shape_SetFriction(box_shape_ID, 0.3f);
    b2Shape_SetRestitution(box_shape_ID, 0.5f);
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

Scenery::Scenery(b2WorldId worldId)
{
    b2BodyDef ground_body_def = b2DefaultBodyDef();
    ground_body_def.position = { (SCREEN_WIDTH / 2.0f) / PIXELS_PER_METER, (SCREEN_HEIGHT - 10.0f) / PIXELS_PER_METER };
    Body_Id = b2CreateBody(worldId, &ground_body_def);
    b2Polygon ground_box = b2MakeBox((SCREEN_WIDTH / 2.0f) / PIXELS_PER_METER, 10.0f / PIXELS_PER_METER);
    b2ShapeDef ground_shape_def = b2DefaultShapeDef();
    b2CreatePolygonShape(Body_Id, &ground_shape_def, &ground_box);
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