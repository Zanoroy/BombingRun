#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>

/**
 * @brief Manages all audio playback including sound effects and music
 * 
 * Uses SDL_mixer for audio playback with support for:
 * - Multiple sound effects channels
 * - Background music
 * - Volume control per category
 * - Sound effect pooling for performance
 */
class AudioManager {
public:
    // Sound effect categories
    enum class SoundEffect {
        EXPLOSION_SMALL,      // Small bombs (100lb-500lb)
        EXPLOSION_MEDIUM,     // Medium bombs (1000lb)
        EXPLOSION_LARGE,      // Large bombs (2000lb-8000lb)
        AIRCRAFT_DESTROYED,   // Bomber/Fighter destroyed
        BULLET_FIRE,          // Bullet fired
        AAA_FIRE,             // AAA gun fire
        BOMB_DROP,            // Bomb released from aircraft
        MENU_CLICK,           // UI click sound
        MENU_HOVER            // UI hover sound
    };

    // Music tracks
    enum class Music {
        MENU,
        GAMEPLAY,
        VICTORY,
        DEFEAT
    };

    AudioManager();
    ~AudioManager();

    // Initialization
    bool initialize();
    void shutdown();

    // Sound effects
    void playSound(SoundEffect effect, float volume = 1.0f);
    void setSoundVolume(float volume); // 0.0 to 1.0
    float getSoundVolume() const { return m_soundVolume; }

    // Music
    void playMusic(Music track, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume); // 0.0 to 1.0
    float getMusicVolume() const { return m_musicVolume; }

    // Master volume
    void setMasterVolume(float volume); // 0.0 to 1.0
    float getMasterVolume() const { return m_masterVolume; }

    // Muting
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }

    // Update (for fading, etc.)
    void update(float deltaTime);

private:
    // Load audio files
    bool loadSoundEffects();
    bool loadMusic();

    // Helper to get actual volume with master and mute
    int getAdjustedVolume(float baseVolume) const;

    // Audio state
    bool m_initialized;
    bool m_muted;
    float m_masterVolume;
    float m_soundVolume;
    float m_musicVolume;

    // Sound effects storage
    std::unordered_map<SoundEffect, Mix_Chunk*> m_soundEffects;
    
    // Music storage
    std::unordered_map<Music, Mix_Music*> m_musicTracks;
    Music m_currentMusic;

    // Audio specs
    static constexpr int AUDIO_FREQUENCY = 44100;
    static constexpr Uint16 AUDIO_FORMAT = AUDIO_S16SYS;
    static constexpr int AUDIO_CHANNELS = 2;
    static constexpr int AUDIO_CHUNK_SIZE = 2048;
    static constexpr int MAX_SOUND_CHANNELS = 32;
};
