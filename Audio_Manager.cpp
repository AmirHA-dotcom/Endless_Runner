//
// Created by amirh on 2025-08-14.
//

#include "Audio_Manager.h"

void Audio_Manager::Init()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
    }
}

void Audio_Manager::LoadSound(const string& name, const string& path)
{
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (chunk != nullptr)
    {
        m_sounds[name] = chunk;
    }
    else
    {
        cerr << "Failed to load sound effect! Mix_Error: " << Mix_GetError() << endl;
    }
}

void Audio_Manager::PlaySound(const std::string& name)
{
    if (m_sounds.find(name) != m_sounds.end())
    {
        Mix_PlayChannel(-1, m_sounds[name], 0);
    }
}

void Audio_Manager::CleanUp()
{
    for (auto const& [name, chunk] : m_sounds)
    {
        Mix_FreeChunk(chunk);
    }
    m_sounds.clear();
    Mix_Quit();
}