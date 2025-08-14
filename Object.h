//
// Created by amirh on 2025-08-12.
//

#ifndef ENDLESS_RUNNER_OBJECT_H
#define ENDLESS_RUNNER_OBJECT_H

#include <box2d/box2d.h>
#include "box2d/collision.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <cstdint>
#include <cstdlib>
#include <ctime>

using namespace std;

// Global Variables
inline int SCREEN_WIDTH, SCREEN_HEIGHT;

const uint16_t PLAYER_CATEGORY = 0x0001;
const uint16_t GROUND_CATEGORY = 0x0002;
const uint16_t OBSTACLE_CATEGORY = 0x0004;

// Classes

class Object
{
protected:
    b2BodyId Body_Id;
    const float PIXELS_PER_METER = 30.0f;

public:
    virtual ~Object() = default;
    virtual void Update(b2WorldId worldId) = 0;
    virtual void Render(SDL_Renderer* renderer, float cameraX) = 0;
};

class Player : public Object
{
private:
    bool is_Dead;
    int m_jumps_Left;
    int MAX_JUMPS = 2;
public:
    explicit Player(b2WorldId WID);
    const float PLAYER_SPEED = 40.0f;

    void Update(b2WorldId worldId) override;
    void Render(SDL_Renderer* renderer, float cameraX) override;

    void Jump();
    void Move_Right();
    void Move_Left();

    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }

    bool IsDead() const { return is_Dead; }
    void SetIsDead(bool isDead) { is_Dead = isDead; }
    float Get_Radius_Meters() const;

    bool Is_On_Ground(b2WorldId worldId);
    bool Can_Jump() const;
};

class Scenery : public Object
{
private:
    float m_Width_Meters;
public:
    explicit Scenery(b2WorldId WID, float startX);
    ~Scenery();

    void Update(b2WorldId worldId) override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
};

class Obstacle : public Object
{
private:
    float m_Width_Px;
    float m_Height_Px;

    bool m_isScored = false;
public:
    explicit Obstacle(b2WorldId worldId, float x, float y, float width, float height);

    ~Obstacle();

    void Update(b2WorldId worldId) override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }

    float GetWidthMeters() const { return m_Width_Px / PIXELS_PER_METER; }
    float GetHeightMeters() const { return m_Height_Px / PIXELS_PER_METER; }

    bool Is_Scored() const { return m_isScored; }
    void Set_Scored(bool scored) { m_isScored = scored; }
};


#endif //ENDLESS_RUNNER_OBJECT_H
