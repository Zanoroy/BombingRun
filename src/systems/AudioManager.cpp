#include "systems/AudioManager.h"
#include <iostream>

AudioManager::AudioManager()
    : m_initialized(false)
    , m_muted(false)
    , m_masterVolume(0.7f)
    , m_soundVolume(1.0f)
    , m_musicVolume(0.5f)
    , m_currentMusic(Music::MENU)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        return false;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(MAX_SOUND_CHANNELS);

    // Set initial volumes
    Mix_VolumeMusic(getAdjustedVolume(m_musicVolume));

    std::cout << "AudioManager initialized successfully" << std::endl;
    std::cout << "  Frequency: " << AUDIO_FREQUENCY << " Hz" << std::endl;
    std::cout << "  Channels: " << AUDIO_CHANNELS << std::endl;
    std::cout << "  Sound channels: " << MAX_SOUND_CHANNELS << std::endl;

    // Load audio files (currently placeholders - files will be added later)
    loadSoundEffects();
    loadMusic();

    m_initialized = true;
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Stop all audio
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

    // Free sound effects
    for (auto& pair : m_soundEffects) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_soundEffects.clear();

    // Free music
    for (auto& pair : m_musicTracks) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_musicTracks.clear();

    // Close SDL_mixer
    Mix_CloseAudio();

    m_initialized = false;
    std::cout << "AudioManager shut down" << std::endl;
}

bool AudioManager::loadSoundEffects() {
    // Note: Audio files will be added to assets/audio/ directory
    // For now, we'll create placeholders and the system will gracefully handle missing files
    
    std::cout << "Loading sound effects..." << std::endl;
    
    // Attempt to load sound files (will return nullptr if not found)
    // This allows the game to run without audio files present
    
    // Explosions
    m_soundEffects[SoundEffect::EXPLOSION_SMALL] = Mix_LoadWAV("assets/audio/explosion_small.wav");
    m_soundEffects[SoundEffect::EXPLOSION_MEDIUM] = Mix_LoadWAV("assets/audio/explosion_medium.wav");
    m_soundEffects[SoundEffect::EXPLOSION_LARGE] = Mix_LoadWAV("assets/audio/explosion_large.wav");
    
    // Aircraft
    m_soundEffects[SoundEffect::AIRCRAFT_DESTROYED] = Mix_LoadWAV("assets/audio/aircraft_destroyed.wav");
    
    // Weapons
    m_soundEffects[SoundEffect::BULLET_FIRE] = Mix_LoadWAV("assets/audio/bullet_fire.wav");
    m_soundEffects[SoundEffect::AAA_FIRE] = Mix_LoadWAV("assets/audio/aaa_fire.wav");
    m_soundEffects[SoundEffect::BOMB_DROP] = Mix_LoadWAV("assets/audio/bomb_drop.wav");
    
    // UI
    m_soundEffects[SoundEffect::MENU_CLICK] = Mix_LoadWAV("assets/audio/menu_click.wav");
    m_soundEffects[SoundEffect::MENU_HOVER] = Mix_LoadWAV("assets/audio/menu_hover.wav");

    int loadedCount = 0;
    for (const auto& pair : m_soundEffects) {
        if (pair.second != nullptr) {
            loadedCount++;
        }
    }

    std::cout << "  Loaded " << loadedCount << "/" << m_soundEffects.size() << " sound effects" << std::endl;
    
    // Return true even if no files loaded - game can run without audio
    return true;
}

bool AudioManager::loadMusic() {
    std::cout << "Loading music tracks..." << std::endl;
    
    // Attempt to load music files
    m_musicTracks[Music::MENU] = Mix_LoadMUS("assets/audio/music_menu.ogg");
    m_musicTracks[Music::GAMEPLAY] = Mix_LoadMUS("assets/audio/music_gameplay.ogg");
    m_musicTracks[Music::VICTORY] = Mix_LoadMUS("assets/audio/music_victory.ogg");
    m_musicTracks[Music::DEFEAT] = Mix_LoadMUS("assets/audio/music_defeat.ogg");

    int loadedCount = 0;
    for (const auto& pair : m_musicTracks) {
        if (pair.second != nullptr) {
            loadedCount++;
        }
    }

    std::cout << "  Loaded " << loadedCount << "/" << m_musicTracks.size() << " music tracks" << std::endl;
    
    return true;
}

void AudioManager::playSound(SoundEffect effect, float volume) {
    if (!m_initialized || m_muted) {
        return;
    }

    auto it = m_soundEffects.find(effect);
    if (it == m_soundEffects.end() || it->second == nullptr) {
        // Sound not loaded, silently skip
        return;
    }

    // Play on first available channel
    int channel = Mix_PlayChannel(-1, it->second, 0);
    if (channel == -1) {
        // No channels available, but this is not critical
        return;
    }

    // Set volume for this channel
    Mix_Volume(channel, getAdjustedVolume(volume * m_soundVolume));
}

void AudioManager::setSoundVolume(float volume) {
    m_soundVolume = std::max(0.0f, std::min(1.0f, volume));
}

void AudioManager::playMusic(Music track, bool loop) {
    if (!m_initialized || m_muted) {
        return;
    }

    auto it = m_musicTracks.find(track);
    if (it == m_musicTracks.end() || it->second == nullptr) {
        // Music not loaded, silently skip
        return;
    }

    // Stop current music if playing
    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(500); // 500ms fade out
    }

    // Play new music
    if (Mix_FadeInMusic(it->second, loop ? -1 : 0, 500) == -1) {
        std::cerr << "Failed to play music: " << Mix_GetError() << std::endl;
        return;
    }

    m_currentMusic = track;
    Mix_VolumeMusic(getAdjustedVolume(m_musicVolume));
}

void AudioManager::stopMusic() {
    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(500);
    }
}

void AudioManager::pauseMusic() {
    if (Mix_PlayingMusic() && !Mix_PausedMusic()) {
        Mix_PauseMusic();
    }
}

void AudioManager::resumeMusic() {
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void AudioManager::setMusicVolume(float volume) {
    m_musicVolume = std::max(0.0f, std::min(1.0f, volume));
    if (m_initialized) {
        Mix_VolumeMusic(getAdjustedVolume(m_musicVolume));
    }
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
    if (m_initialized) {
        Mix_VolumeMusic(getAdjustedVolume(m_musicVolume));
    }
}

void AudioManager::setMuted(bool muted) {
    m_muted = muted;
    if (m_initialized) {
        if (muted) {
            Mix_VolumeMusic(0);
            Mix_Volume(-1, 0); // Mute all channels
        } else {
            Mix_VolumeMusic(getAdjustedVolume(m_musicVolume));
            // Note: Individual channel volumes will be set when sounds play
        }
    }
}

void AudioManager::update(float deltaTime) {
    // Currently no per-frame updates needed
    // This could be used for music crossfading, sound ducking, etc.
}

int AudioManager::getAdjustedVolume(float baseVolume) const {
    if (m_muted) {
        return 0;
    }
    
    // SDL_mixer volume range is 0-128
    float adjustedVolume = baseVolume * m_masterVolume;
    return static_cast<int>(adjustedVolume * MIX_MAX_VOLUME);
}
