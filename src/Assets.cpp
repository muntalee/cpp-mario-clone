#include <cassert>
#include <fstream>
#include <iostream>
#include "Assets.h"

Assets::Assets() = default;

void Assets::loadFromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Could not load " << path << " file!\n";
        exit(-1);
    }

    std::string assetType;
    while (file >> assetType)
    {
        if (assetType == "#")
        {
            continue;
        }
        else if (assetType == "Texture")
        {
            std::string name;
            std::string imageFile;
            file >> name >> imageFile;
            addTexture(name, imageFile);
        }
        else if (assetType == "Animation")
        {
            std::string aniName;
            std::string texName;
            int frames, speed;
            file >> aniName >> texName >> frames >> speed;
            const sf::Texture &tex = getTexture(texName);
            addAnimation(aniName, Animation(aniName, tex, frames, speed));
        }
        else if (assetType == "Font")
        {
            std::string fontName;
            std::string fontPath;
            file >> fontName >> fontPath;
            addFont(fontName, fontPath);
        }
        else
        {
            std::cerr << "Incorrect asset type: " << assetType << "\n";
            exit(-1);
        }
    }
}

void Assets::addTexture(const std::string &name, const std::string &path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        std::cerr << "Could not load image: " << path << "!\n";
        exit(-1);
    }
    m_textureMap[name] = texture;
}

void Assets::addAnimation(const std::string &name, const Animation &animation)
{
    m_animationMap[name] = animation;
}

void Assets::addFont(const std::string &name, const std::string &path)
{
    sf::Font font;
    if (!font.loadFromFile(path))
    {
        std::cerr << "Could not load font: " << path << "\n";
        exit(-1);
    }
    m_fontMap[name] = font;
}

const sf::Texture &Assets::getTexture(const std::string &name) const
{
    auto it = m_textureMap.find(name);
    if (it != m_textureMap.end())
    {
        return it->second;
    }
    else
    {
        std::cerr << "Texture not found: " << name << "\n";
        exit(-1); // or handle the error appropriately
    }
}

const Animation &Assets::getAnimation(const std::string &name) const
{
    auto it = m_animationMap.find(name);
    if (it != m_animationMap.end())
    {
        return it->second;
    }
    else
    {
        std::cerr << "Animation not found: " << name << "\n";
        exit(-1); // or handle the error appropriately
    }
}

const sf::Font &Assets::getFont(const std::string &name) const
{
    auto it = m_fontMap.find(name);
    if (it != m_fontMap.end())
    {
        return it->second;
    }
    else
    {
        std::cerr << "Font not found: " << name << "\n";
        exit(-1); // or handle the error appropriately
    }
}
