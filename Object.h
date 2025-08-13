//
// Created by amirh on 2025-08-12.
//

#ifndef ENDLESS_RUNNER_OBJECT_H
#define ENDLESS_RUNNER_OBJECT_H

#include <box2d/box2d.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <deque>

using namespace std;

// Global Variables
inline int SCREEN_WIDTH, SCREEN_HEIGHT;


class Object
{
protected:
    b2BodyId Body_Id;
    const float PIXELS_PER_METER = 30.0f;

public:
    virtual ~Object() = default;
    virtual void Update() = 0;
    virtual void Render(SDL_Renderer* renderer, float cameraX) = 0;
};

class Player : public Object
{
public:
    explicit Player(b2WorldId WID);
    const float PLAYER_SPEED = 10.0f;

    void Update() override;
    void Render(SDL_Renderer* renderer, float cameraX) override;

    void Jump();
    void Move_Right();
    void Move_Left();

    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }
};

class Scenery : public Object
{
private:
    float m_Width_Meters;
public:
    explicit Scenery(b2WorldId WID, float startX);
    ~Scenery();

    void Update() override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
};

#endif //ENDLESS_RUNNER_OBJECT_H
