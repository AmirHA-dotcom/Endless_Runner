//
// Created by amirh on 2025-08-15.
//

#include "Asset_Manager.h"

void Asset_Manager::LoadTexture(const std::string& name, const std::string& path, SDL_Renderer* renderer)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr)
    {
        std::cerr << "Failed to load texture: " << path << " | Error: " << IMG_GetError() << std::endl;
    }
    else
    {
        m_textures[name] = texture;
    }
}

SDL_Texture* Asset_Manager::GetTexture(const std::string& name)
{
    if (m_textures.find(name) != m_textures.end())
    {
        return m_textures[name];
    }
    return nullptr;
}

void Asset_Manager::CleanUp()
{
    for (auto const& [name, texture] : m_textures)
    {
        SDL_DestroyTexture(texture);
    }
    m_textures.clear();
}