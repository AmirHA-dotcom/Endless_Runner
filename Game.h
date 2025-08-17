//
// Created by amirh on 2025-08-09.
//

#ifndef ENDLESS_RUNNER_GAME_H
#define ENDLESS_RUNNER_GAME_H

#include "Object.h"

struct Skin {
    string id;            // The key used in the AssetManager (e.g., "player_default")
    string displayName;
    int price;
    bool isUnlocked = false;
};

enum class STATE { MAIN_MENU, PLAYING, GAME_OVER, SHOP };

class Game
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    b2WorldId World_Id;
    const char* FONT = "D:/Fonts/Roboto/static/Roboto-Regular.ttf";
    const float PIXELS_PER_METER = 30.0f;
    TTF_Font* font_large;
    TTF_Font* font_regular;

    bool running = true;

    float cameraX = 0.0f;

    unique_ptr<Player> m_Player;
    deque<unique_ptr<Scenery>> m_Ground_Segments;
    deque<unique_ptr<Obstacle>> m_Obstacles;
    deque<unique_ptr<PowerUp>> m_powerUps;
    deque<unique_ptr<Coin>> m_coins;

    float m_Obstacle_Spawn_Timer = 0.0f;

    long int m_score = 0;

    STATE m_current_State;
    vector<int> m_high_Scores;

    vector<string> small_obstacle_skins;
    vector<string> tall_obstacle_skins;
    vector<string> wide_obstacle_skins;

    vector<vector<SDL_Point>> m_starLayers;

    long int current_coins = 0;
    long int m_totalCoins = 0;

    float m_tutorialTextTimer = 5.0f;

    int m_uiCoinFrameCount = 0;
    int m_uiCoinCurrentFrame = 0;
    int m_uiCoinFrameWidth = 0;
    int m_uiCoinFrameHeight = 0;
    float m_uiCoinAnimTimer = 0.0f;
    float m_uiCoinAnimSpeed = 0.1f;

    std::vector<Skin> m_allSkins;
    int m_currentSkinIndex = 0; // Which skin is currently selected in the shop
    std::string m_equippedSkinId = "player_default";
public:
    Game();
    void Run();

    void Generate_Initial_Ground();
    void Update_Ground();
    void Update_Spawning(float deltaTime);
    void Update_Score();
    void Render_UI();
    void Reset_Game();
    void Render_Playing();
    void Update_Playing(float timeStep);
    void Update_GameOver(const SDL_Event& event);
    void Render_GameOver();
    void Update_MainMenu(const SDL_Event& event);
    void Render_MainMenu();
    void HandleEvents_Playing(const SDL_Event& event);
    void HandleEvents_MainMenu(const SDL_Event& event);
    void HandleEvents_GameOver(const SDL_Event& event);
    void Save_Scores();
    void Load_Scores();
    void Update_High_Scores();
    void Load_Assets();
    void GenerateStars();
    void RenderStarfield();
    void SaveWallet();
    void LoadWallet();
    void HandleEvents_Shop(const SDL_Event& event);
    void Render_Shop();
    void InitializeSkins();
    void UpdateShop();
};


#endif //ENDLESS_RUNNER_GAME_H
