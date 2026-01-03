#include "systems/TextureManager.h"
#include <iostream>

namespace BombingRun {

TextureManager::TextureManager()
    : m_renderer(nullptr)
    , m_initialized(false)
{
}

TextureManager::~TextureManager() {
    cleanup();
}

TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

bool TextureManager::initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize SDL_image with PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }

    m_initialized = true;
    std::cout << "TextureManager initialized successfully" << std::endl;
    return true;
}

void TextureManager::setRenderer(SDL_Renderer* renderer) {
    m_renderer = renderer;
}

bool TextureManager::loadTexture(const std::string& id, const std::string& filepath) {
    if (!m_renderer) {
        std::cerr << "Cannot load texture: renderer not set" << std::endl;
        return false;
    }

    // Check if already loaded
    if (m_textures.find(id) != m_textures.end()) {
        std::cout << "Texture '" << id << "' already loaded" << std::endl;
        return true;
    }

    // Load image as surface
    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (!surface) {
        std::cerr << "Failed to load image '" << filepath << "': " << IMG_GetError() << std::endl;
        return false;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Failed to create texture from '" << filepath << "': " << SDL_GetError() << std::endl;
        return false;
    }

    m_textures[id] = texture;
    std::cout << "Loaded texture '" << id << "' from '" << filepath << "'" << std::endl;
    return true;
}

SDL_Texture* TextureManager::getTexture(const std::string& id) {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    std::cerr << "Texture '" << id << "' not found" << std::endl;
    return nullptr;
}

void TextureManager::draw(const std::string& id, int x, int y, int width, int height, double angle) {
    SDL_Texture* texture = getTexture(id);
    if (!texture || !m_renderer) {
        return;
    }

    SDL_Rect destRect = {x, y, width, height};
    SDL_RenderCopyEx(m_renderer, texture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
}

void TextureManager::drawFrame(const std::string& id, const SDL_Rect& srcRect, const SDL_Rect& destRect, double angle) {
    SDL_Texture* texture = getTexture(id);
    if (!texture || !m_renderer) {
        return;
    }

    SDL_RenderCopyEx(m_renderer, texture, &srcRect, &destRect, angle, nullptr, SDL_FLIP_NONE);
}

void TextureManager::unloadTexture(const std::string& id) {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        SDL_DestroyTexture(it->second);
        m_textures.erase(it);
        std::cout << "Unloaded texture '" << id << "'" << std::endl;
    }
}

void TextureManager::cleanup() {
    for (auto& pair : m_textures) {
        SDL_DestroyTexture(pair.second);
    }
    m_textures.clear();

    if (m_initialized) {
        IMG_Quit();
        m_initialized = false;
        std::cout << "TextureManager cleaned up" << std::endl;
    }
}

} // namespace BombingRun
