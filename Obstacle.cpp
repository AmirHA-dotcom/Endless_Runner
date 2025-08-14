//
// Created by amirh on 2025-08-14.
//

#include "Obstacle.h"

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