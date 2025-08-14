//
// Created by amirh on 2025-08-14.
//

#ifndef ENDLESS_RUNNER_OBSTACLE_H
#define ENDLESS_RUNNER_OBSTACLE_H

#include "Object.h"

class Obstacle : public Object
{
private:
    float m_Width_Px;
    float m_Height_Px;
public:
    explicit Obstacle(b2WorldId worldId, float x, float y);

    ~Obstacle();

    void Update() override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
};


#endif //ENDLESS_RUNNER_OBSTACLE_H
