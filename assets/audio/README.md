# Audio Assets for Bombing Run

This directory contains audio files for the game. The AudioManager will gracefully handle missing files - the game will run without audio if files are not present.

## Required Sound Effects (.wav format recommended)

### Explosions
- **explosion_small.wav** - Small bomb explosions (100lb-250lb)
- **explosion_medium.wav** - Medium bomb explosions (500lb-1000lb)
- **explosion_large.wav** - Large bomb explosions (2000lb-8000lb)

### Aircraft
- **aircraft_destroyed.wav** - Aircraft/fighter jet destruction sound

### Weapons
- **bullet_fire.wav** - Fighter jet bullet fire
- **aaa_fire.wav** - AAA gun fire sound
- **bomb_drop.wav** - Bomb release sound

### UI
- **menu_click.wav** - Menu button click
- **menu_hover.wav** - Menu button hover (optional)

## Music Tracks (.ogg format recommended)

- **music_menu.ogg** - Main menu background music
- **music_gameplay.ogg** - Gameplay background music
- **music_victory.ogg** - Victory screen music
- **music_defeat.ogg** - Defeat screen music

## Audio Specifications

### Sound Effects
- **Format:** WAV (uncompressed) for best compatibility
- **Sample Rate:** 44100 Hz recommended
- **Channels:** Mono or Stereo
- **Bit Depth:** 16-bit recommended

### Music
- **Format:** OGG Vorbis (compressed, good quality/size ratio)
- **Sample Rate:** 44100 Hz recommended
- **Channels:** Stereo
- **Bitrate:** 128-192 kbps recommended

## Volume Guidelines

The game automatically adjusts volumes:
- **Small explosions:** 60% volume
- **Medium explosions:** 80% volume  
- **Large explosions:** 100% volume
- **Aircraft destroyed:** 90% volume
- **Bullet fire:** 30% volume
- **Menu sounds:** 50-70% volume

Master volume is set to 70% by default and can be adjusted in-game.

## Sources for Free Audio

- **Freesound.org** - Community sound effects library
- **OpenGameArt.org** - Open source game assets
- **Incompetech.com** - Royalty-free music by Kevin MacLeod
- **ZapSplat.com** - Free sound effects for creators

## Creating Your Own

You can create/record your own sounds or use audio editing software like:
- **Audacity** (free, cross-platform)
- **LMMS** (free music production)
- **Bfxr** (8-bit sound effect generator)

## Note

The game is fully functional without audio files. Audio will simply not play if files are missing. This allows for:
- Testing without complete audio assets
- Custom audio replacements by users
- Different audio packs/mods
