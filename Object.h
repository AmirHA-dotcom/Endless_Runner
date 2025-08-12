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

class Object
{
protected:
    b2BodyId Body_Id;

public:
    virtual ~Object() = default;
    virtual void Update() = 0;
    virtual void Render(SDL_Renderer* renderer) = 0;
};

class Player : public Object
{
public:
    explicit Player(b2WorldId WID);

    void Update() override;
    void Render(SDL_Renderer* renderer) override;
};

class Scenery : public Object
{
public:
    explicit Scenery(b2WorldId WID);

    void Update() override;
    void Render(SDL_Renderer* renderer) override;
};

#endif //ENDLESS_RUNNER_OBJECT_H
