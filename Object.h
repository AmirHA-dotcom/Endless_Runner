//
// Created by amirh on 2025-08-12.
//

#ifndef ENDLESS_RUNNER_OBJECT_H
#define ENDLESS_RUNNER_OBJECT_H

#include "Audio_Manager.h"
#include "Asset_Manager.h"

// Global Variables

inline int SCREEN_WIDTH, SCREEN_HEIGHT;

const uint16_t PLAYER_CATEGORY = 0x0001;
const uint16_t GROUND_CATEGORY = 0x0002;
const uint16_t OBSTACLE_CATEGORY = 0x0004;



enum class PowerUpType { EXTRA_JUMP, DOUBLE_SCORE };

// Classes

class Object
{
protected:
    b2BodyId Body_Id  = b2_nullBodyId;
    const float PIXELS_PER_METER = 30.0f;
    SDL_Texture* m_texture;
    string m_currentSkin = "";
public:
    virtual ~Object();
    virtual void Update(b2WorldId worldId, float deltaTime, int score) = 0;
    virtual void Render(SDL_Renderer* renderer, float cameraX) = 0;
};

// Player

class Player : public Object
{
private:
    bool is_Dead;
    int m_jumps_Left;
    int MAX_JUMPS = 2;
    float m_doubleScoreTimer = 0.0f;
    float m_extraJumpTimer = 0.0f;

    float PLAYER_SPEED = 20.0f;
    const float BASE_SPEED = 20.0f;
    const float MAX_SPEED = 100.0f;

    // Animations
    enum class AnimationState { RUNNING, JUMPING, FALLING };
    AnimationState m_animState;
    int m_animRow;
    int m_frameCount;      // Total number of frames in the animation
    int m_currentFrame;    // The current frame to draw (e.g., 0, 1, 2...)
    int m_frameWidth;      // The width of a single frame in the sprite sheet
    int m_frameHeight;     // The height of a single frame
    float m_animTimer;       // Timer to control animation speed
    float m_animSpeed;       // How long each frame is displayed (in seconds)
    void SetAnimation(AnimationState state);

public:
    explicit Player(b2WorldId WID);

    void Update(b2WorldId worldId, float deltaTime, int score) override;
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

    void Reset();

    void ActivatePowerUp(PowerUpType type);
    bool HasExtraJump() { return m_extraJumpTimer > 0.0f; }
    bool HasDoubleScore() { return m_doubleScoreTimer > 0.0f; }
    float get_extra_jump_timer() { return m_extraJumpTimer; }
    float get_double_score_timer() { return m_doubleScoreTimer; }
    float GetWidthMeters() const { return 50.0f / PIXELS_PER_METER; }
    float GetHeightMeters() const { return 50.0f / PIXELS_PER_METER; }
};

// Scenery

class Scenery : public Object
{
private:
    float m_Width_Meters;
public:
    explicit Scenery(b2WorldId WID, float startX, SDL_Texture* texture);
    ~Scenery();

    void Update(b2WorldId worldId, float deltaTime, int score) override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
};

// Obstacles

class Obstacle : public Object
{
private:
    float m_Width_Px;
    float m_Height_Px;

    bool m_isScored = false;
public:
    explicit Obstacle(b2WorldId worldId, float x, float y, float width, float height, SDL_Texture* texture);

    ~Obstacle();

    void Update(b2WorldId worldId, float deltaTime, int score) override;
    void Render(SDL_Renderer* renderer, float cameraX) override;
    float Get_Right_EdgeX() const;
    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }

    float GetWidthMeters() const { return m_Width_Px / PIXELS_PER_METER; }
    float GetHeightMeters() const { return m_Height_Px / PIXELS_PER_METER; }

    bool Is_Scored() const { return m_isScored; }
    void Set_Scored(bool scored) { m_isScored = scored; }
};

// Power Ups

class PowerUp : public Object
{
private:
    PowerUpType m_type;

    int m_frameCount;
    int m_currentFrame;
    int m_frameWidth;
    int m_frameHeight;
    float m_animTimer;
    float m_animSpeed;
public:
    explicit PowerUp(b2WorldId worldId, PowerUpType type, float x, float y);
    void Render(SDL_Renderer* renderer, float cameraX) override;
    void Update(b2WorldId worldId, float deltaTime, int score) override;

    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }

    PowerUpType GetType() const { return m_type; }
};

// Health

class Health : public Object
{
private:
    int m_frameCount;
    int m_currentFrame;
    int m_frameWidth;
    int m_frameHeight;
    float m_animTimer;
    float m_animSpeed;
public:
    explicit Health(b2WorldId worldId, PowerUpType type, float x, float y);
    void Render(SDL_Renderer* renderer, float cameraX) override;
    void Update(b2WorldId worldId, float deltaTime, int score) override;

    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }
};


// Coins

class Coin : public Object
{
private:
    int m_frameCount;
    int m_currentFrame;
    int m_frameWidth;
    int m_frameHeight;
    float m_animTimer;
    float m_animSpeed;
public:
    explicit Coin(b2WorldId worldId, float x, float y);
    void Render(SDL_Renderer* renderer, float cameraX) override;
    void Update(b2WorldId worldId, float deltaTime, int score) override;

    b2Vec2  get_position() { return b2Body_GetPosition(Body_Id); }
};

#endif //ENDLESS_RUNNER_OBJECT_H
