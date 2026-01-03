#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace BombingRun {

/**
 * @brief Manages loading, caching, and cleanup of textures
 * 
 * Singleton pattern to provide global access to texture resources.
 * Automatically handles SDL_image initialization and cleanup.
 */
class TextureManager {
public:
    static TextureManager& getInstance();

    // Delete copy and move constructors
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    /**
     * @brief Initialize SDL_image library
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Set the renderer for texture creation
     * @param renderer SDL renderer
     */
    void setRenderer(SDL_Renderer* renderer);

    /**
     * @brief Load a texture from file
     * @param id Unique identifier for the texture
     * @param filepath Path to image file
     * @return true if successful, false otherwise
     */
    bool loadTexture(const std::string& id, const std::string& filepath);

    /**
     * @brief Get a loaded texture
     * @param id Texture identifier
     * @return Pointer to SDL_Texture or nullptr if not found
     */
    SDL_Texture* getTexture(const std::string& id);

    /**
     * @brief Draw a texture to screen
     * @param id Texture identifier
     * @param x X position
     * @param y Y position
     * @param width Width to draw
     * @param height Height to draw
     * @param angle Rotation angle in degrees
     */
    void draw(const std::string& id, int x, int y, int width, int height, double angle = 0.0);

    /**
     * @brief Draw a portion of a texture (for sprite sheets)
     * @param id Texture identifier
     * @param srcRect Source rectangle in texture
     * @param destRect Destination rectangle on screen
     * @param angle Rotation angle in degrees
     */
    void drawFrame(const std::string& id, const SDL_Rect& srcRect, const SDL_Rect& destRect, double angle = 0.0);

    /**
     * @brief Unload a specific texture
     * @param id Texture identifier
     */
    void unloadTexture(const std::string& id);

    /**
     * @brief Unload all textures and cleanup
     */
    void cleanup();

private:
    TextureManager();
    ~TextureManager();

    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, SDL_Texture*> m_textures;
    bool m_initialized;
};

} // namespace BombingRun
