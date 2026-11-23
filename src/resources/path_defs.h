
#ifndef MIXED_SOUND_TYPE
#define MIXED_SOUND_TYPE "mixedSoundType"
#endif

#ifndef SONG_TYPE
#define SONG_TYPE "songType"
#endif

#ifndef MATERIAL_TYPE
#define MATERIAL_TYPE "materialType"
#endif

#ifndef WEAPON_TYPE
#define WEAPON_TYPE "weaponType"
#endif

// GAMEPLAY
////////////////////////////////////////////////////////////////////////////////////

DEFINE_FILE_RESOURCE(DEFAULT_SOUND, "../gameresources/sound/q009/weapswitch.ogg")
DEFINE_FILE_RESOURCE(WEAPON_SWITCH, "../gameresources/sound/q009/weapswitch.ogg")
DEFINE_FILE_RESOURCE(EXPLOSION, "../gameresources/sound/q009/rlauncher3.ogg")
DEFINE_FILE_RESOURCE(HITMARKER,  "./res/sounds/sample.wav")
DEFINE_FILE_RESOURCE(TELEPORT_SOUND, "../gameresources/sound/q009/teleport.ogg")
DEFINE_FILE_RESOURCE(ACTIVATE_SOUND, "../gameresources/sound/click.wav")

DEFINE_RESOURCE(BREAKABLE,  "ball/entity/orbpickup", MIXED_SOUND_TYPE )

// Ball Sounds
DEFINE_RESOURCE(BALL_JUMP, "ball/jump", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(BALL_GRAVITYWELL, "effects/gravitywell", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(BALL_ROLLING,  "ball/rolling", MIXED_SOUND_TYPE )


// Sounds
DEFINE_RESOURCE(FIREWORK, "effects/firework", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_SCREENS_TERMINAL, "screens/terminal", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_SCREENS_MENUCLICK, "screens/menuclick", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_EVENT_WARPEND, "ball/events/warpend", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_EVENT_LEVELCOMPLETE, "ball/events/levelcomplete", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_EVENT_WARPSTART, "ball/events/warpstart", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_SCREEN_NEWGAME, "ball/screens/newgame", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_SCREEN_CONTINUE, "ball/screens/continue", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_SCREEN_WORLDPRESS, "ball/screens/worldpress", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_SCREEN_LEVELSELECT, "ball/screens/levelselect", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_GETPOWERUP, "ball/getpowerup", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_RHYTHM_START_SOUL, "ball/rhythm/start_soul", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_RHYTHM_PRESS_NOTE, "ball/rhythm/press_note", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_BALL_CHANGESPIRIT, "ball/changespirit", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_ARCADE_TENNIS_HIT, "arcade/tennis/hit", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_ARCADE_INVADERS_ENEMYKILL, "arcade/invaders/enemykill", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_ARCADE_INVADERS_SHIPSHOOT, "arcade/invaders/shipshoot", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_ARCADE_INVADERS_STARTMENU, "arcade/invaders/startmenu", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_EFFECTS_EXPLOSION, "effects/explosion", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_EFFECTS_TELEPORT, "effects/teleport", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_INTERACTION_ACTIVATE, "interaction/activate", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_INTERACTION_DEACTIVATE, "interaction/deactivate", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_INTERACTION_COLLISION, "interaction/collision", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_PLAYERHURT, "fps/playerhurt", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_ZOOMIN, "fps/zoomin", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_ZOOMOUT, "fps/zoomout", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_JUMP, "fps/jump", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_LAND, "fps/land", MIXED_SOUND_TYPE)
DEFINE_RESOURCE(SOUND_FPS_WALK, "fps/walk", MIXED_SOUND_TYPE)

// Particles
DEFINE_RESOURCE(PARTICLE_ELECTRIC, "electric")


// Graphical Features
////////////////////////////////////////////////////////////////////////////////////
DEFINE_FILE_RESOURCE(WATER_TEXTURE, "./res/heightmaps/dunes_low.jpg")  // "../gameresources/build/textures/lava.png"

// Enemies
////////////////////////////////////////////////////////////////////////////////////
DEFINE_FILE_RESOURCE(AI_TV_HAPPY_FACE, "../gameresources/textures/enemies/happy.png")

// CUTSCENES
////////////////////////////////////////////////////////////////////////////////////
DEFINE_DIRECTORY_RESOURCE(SHADER_STORYBOARD, "../afterworld/shaders/storyboard")
DEFINE_DIRECTORY_RESOURCE(SHADER_BALL_DIRECTORY, "../afterworld/shaders/ball")
DEFINE_DIRECTORY_RESOURCE(SHADER_BALL_EYE_DIRECTORY, "../afterworld/shaders/ball-eye")
DEFINE_DIRECTORY_RESOURCE(SHADER_GRASS_DIRECTORY, "../afterworld/shaders/grass")
DEFINE_DIRECTORY_RESOURCE(SHADER_ARCADE_DIRECTORY, "../afterworld/shaders/arcade")
DEFINE_DIRECTORY_RESOURCE(SHADER_LETTERBOX_DIRECTORY, "../afterworld/shaders/letterbox")
DEFINE_DIRECTORY_RESOURCE(SHADER_WATER_DIRECTORY, "../afterworld/shaders/water")
DEFINE_FILE_RESOURCE(IMAGE_CUTSCENE_HELL, "../afterworld/design/storyboard/hell.jpg")
DEFINE_FILE_RESOURCE(SHADER_BALL_FRAGMENT, "../afterworld/shaders/ball/fragment.glsl")
DEFINE_FILE_RESOURCE(SHADER_BALL_VERTEX, "../afterworld/shaders/ball/vertex.glsl")
DEFINE_FILE_RESOURCE(SHADER_BALL_EYE_FRAGMENT, "../afterworld/shaders/ball-eye/fragment.glsl")
DEFINE_FILE_RESOURCE(SHADER_BALL_EYE_VERTEX, "../afterworld/shaders/ball-eye/vertex.glsl")
DEFINE_FILE_RESOURCE(SHADER_GRASS_FRAGMENT, "../afterworld/shaders/grass/fragment.glsl")
DEFINE_FILE_RESOURCE(SHADER_GRASS_VERTEX, "../afterworld/shaders/grass/vertex.glsl")
DEFINE_FILE_RESOURCE(SHADER_FUR_FRAGMENT, "../afterworld/shaders/fur_shell/fragment.glsl")
DEFINE_FILE_RESOURCE(SHADER_FUR_VERTEX, "../afterworld/shaders/fur_shell/vertex.glsl")
DEFINE_FILE_RESOURCE(SHADER_FUR_GEOMETRY, "../afterworld/shaders/fur_shell/geometry.glsl")
DEFINE_FILE_RESOURCE(SHADER_WATER_FRAGMENT, "../afterworld/shaders/water/fragment.glsl")
DEFINE_FILE_RESOURCE(SHADER_WATER_VERTEX, "../afterworld/shaders/water/vertex.glsl")

// Directly loaded particle effects
DEFINE_FILE_RESOURCE(PARTICLE_SPIRIT_WHITE, "./res/particles/spirit-white.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_ELECTRIC_FILE, "./res/particles/electric.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_BACKGROUNDS, "./res/particles/backgrounds.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_WARP, "./res/particles/warp.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_RAIN, "./res/particles/rain2.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_SPLASH, "./res/particles/splash.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_FIRE, "./res/particles/fire.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_FOG, "./res/particles/fog.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_SPARKS, "./res/particles/sparks.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_FIREWORK, "./res/particles/firework.efkefc")
DEFINE_FILE_RESOURCE(PARTICLE_LASER, "./res/particles/Laser02.efkefc")

// UI
////////////////////////////////////////////////////////////////////////////////////
DEFINE_FILE_RESOURCE(UI_KEYBOARD_IMAGE, "../gameresources/build/misc/keyboard.png")
DEFINE_FILE_RESOURCE(UI_ZOOM_IMAGE, "../gameresources/ui/zoom.png")
DEFINE_FILE_RESOURCE(TERMINAL_BACKGROUND, "../gameresources/build/textures/terminals/terminal.png")
DEFINE_FILE_RESOURCE(UI_CROSSHAIR, "./res/textures/crosshairs/crosshair029.png")
DEFINE_FILE_RESOURCE(UI_TEST_GRADIENT, "./res/textures/testgradient.png")
DEFINE_FILE_RESOURCE(UI_HEXGLOW, "./res/textures/hexglow.png")
DEFINE_FILE_RESOURCE(TEXTURE_EVILPATTERN, "../gameresources/build/textures/evilpattern.png")
DEFINE_FILE_RESOURCE(FONT_OCR, "./res/fonts/ocr.ttf")

// Directly loaded resources
DEFINE_FILE_RESOURCE(MIDI_NEVERGIVEYOUP, "../gameresources/sound/midi/nevergiveyouup.mid")
DEFINE_FILE_RESOURCE(MODEL_SCRAPGUN, "../gameresources/build/weapons/scrapgun.gltf")
DEFINE_FILE_RESOURCE(MODEL_ELECTROGUN, "../gameresources/build/weapons/electrogun.gltf")
DEFINE_FILE_RESOURCE(MODEL_FORK, "../gameresources/build/weapons/fork.gltf")
DEFINE_FILE_RESOURCE(MODEL_BALL, "../gameresources/build/characters/ball.gltf")
DEFINE_FILE_RESOURCE(MODEL_WARP, "../gameresources/build/uncategorized/warp.gltf")
DEFINE_FILE_RESOURCE(MODEL_ARCADE, "../gameresources/build/uncategorized/arcade.gltf")
DEFINE_FILE_RESOURCE(MODEL_PLANE, "../gameresources/build/primitives/plane_xy_1x1.gltf")
DEFINE_FILE_RESOURCE(TEXTURE_BACKGROUND, "../gameresources/textures/backgrounds/test3.png")
DEFINE_FILE_RESOURCE(TEXTURE_MOONMAN, "../gameresources/build/textures/moonman.jpg")
DEFINE_FILE_RESOURCE(TEXTURE_CYBERGUY, "./res/textures/cyberguy2.png")
DEFINE_FILE_RESOURCE(TEXTURE_WATER, "./res/textures/water.jpg")
DEFINE_FILE_RESOURCE(TEXTURE_GLASSBROKEN, "./res/textures/glassbroken.png")
DEFINE_FILE_RESOURCE(TEXTURE_WOOD, "./res/textures/wood.jpg")
DEFINE_FILE_RESOURCE(TEXTURE_BALL_EYE_BASE, "../afterworld/data/textures/ball/eye_base.png")
DEFINE_FILE_RESOURCE(TEXTURE_BALL_EYE_PUPIL, "../afterworld/data/textures/ball/eye_pupil.png")
DEFINE_FILE_RESOURCE(BALL_CONFIG, "../afterworld/data/config/ball/ball.json")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_JUMP, "../gameresources/build/textures/ballgame/jump.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_DASH, "../gameresources/build/textures/ballgame/dash.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_BOUNCE, "../gameresources/build/textures/ballgame/bounce.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_BOUNCE_NORMAL, "../gameresources/build/textures/ballgame/bounce.normal.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_TELEPORT, "../gameresources/build/textures/ballgame/teleport.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_TELEPORT_NORMAL, "../gameresources/build/textures/ballgame/teleport.normal.png")
DEFINE_FILE_RESOURCE(TEXTURE_POWERUP_NONE, "../gameresources/build/textures/ballgame/none.png")

// Ball Game Compiler
DEFINE_FILE_RESOURCE(LASER_MODEL, "../gameresources/build/objtypes/laser.gltf")
DEFINE_FILE_RESOURCE(GRAVITYHOLE_MODEL, "../gameresources/build/objtypes/gravityhole.gltf")

DEFINE_FILE_RESOURCE(POWERUP_MODEL, "../gameresources/build/objtypes/powerup.gltf")
DEFINE_FILE_RESOURCE(POWERUP_MODEL_JUMP, "../gameresources/build/objtypes/powerup.gltf")
DEFINE_FILE_RESOURCE(POWERUP_MODEL_DASH, "../gameresources/build/objtypes/powerup.gltf")
DEFINE_FILE_RESOURCE(POWERUP_MODEL_TELEPORT, "../gameresources/build/objtypes/powerup.gltf")
DEFINE_FILE_RESOURCE(POWERUP_MODEL_INVINCIBILITY, "../gameresources/build/objtypes/powerup.gltf")

DEFINE_FILE_RESOURCE(MUSHROOM, "../gameresources/build/uncategorized/metalmushroom.gltf")
DEFINE_FILE_RESOURCE(SOUL_HOLDER, "../gameresources/build/uncategorized/soulspawn.gltf")
DEFINE_FILE_RESOURCE(SHARD, "../gameresources/build/primitives/sphere.gltf")
DEFINE_FILE_RESOURCE(GEM_MODEL, "../gameresources/build/uncategorized/crystal.gltf")

// ARCADE 
////////////////////////////////////////////////////////////////////////////////////

// Invaders
// -----------------------------------------------------------------
DEFINE_FILE_RESOURCE(GUNSHOT, "../gameresources/sound/q009/minigun2.ogg")

DEFINE_FILE_RESOURCE(INVADERS_SHIP, "../gameresources/textures/arcade/invaders/ship.png")
DEFINE_FILE_RESOURCE(INVADERS_BACKGROUND, "../gameresources/textures/arcade/invaders/background.png")
DEFINE_FILE_RESOURCE(INVADERS_PETE, "../gameresources/textures/arcade/invaders/pete.png")

// Tennis
// -----------------------------------------------------------------
DEFINE_FILE_RESOURCE(TENNIS_HIT_SOUND, "./res/sounds/thud.wav")
DEFINE_FILE_RESOURCE(TENNIS_BALL_IMAGE, "../gameresources/textures/arcade/invaders/pete.png")

// Rhythm
// -----------------------------------------------------------------
DEFINE_FILE_RESOURCE(RHYTHM_LEFT, "../gameresources/textures/arcade/rhythm/arrow-left.png")
DEFINE_FILE_RESOURCE(RHYTHM_RIGHT, "../gameresources/textures/arcade/rhythm/arrow-right.png")
DEFINE_FILE_RESOURCE(RHYTHM_UP, "../gameresources/textures/arcade/rhythm/arrow-up.png")
DEFINE_FILE_RESOURCE(RHYTHM_DOWN, "../gameresources/textures/arcade/rhythm/arrow-down.png")

// Helicopter
// -----------------------------------------------------------------
DEFINE_FILE_RESOURCE(HELICOPTER_BACKGROUND,  "../gameresources/textures/lava.png")
DEFINE_FILE_RESOURCE(HELICOPTER_MAIN_SOUND,  "../gameresources/sound/rain.wav")


// Interact
// -----------------------------------------------------------------
DEFINE_FILE_RESOURCE(ARCADE_INTERACT_IMAGE, "../gameresources/textures/arcade/invaders/background.png")

// Example missing resource
DEFINE_FILE_RESOURCE(FAKE_RESOURCE, "../gameresources/textures/arcade/fake-resource.txt")

// Directly used directories
DEFINE_DIRECTORY_RESOURCE(SKYBOX_DESERT_DIRECTORY, "./res/textures/skyboxs/desert")
DEFINE_DIRECTORY_RESOURCE(SKYBOX_STORM_DIRECTORY, "../gameresources/skybox/storm")
DEFINE_DIRECTORY_RESOURCE(SKYBOX_SPACE_DIRECTORY, "../gameresources/skybox/space1")
DEFINE_DIRECTORY_RESOURCE(SQL_DIRECTORY, "../afterworld/data/sql")
DEFINE_DIRECTORY_RESOURCE(PLAYLIST_DIRECTORY, "../afterworld/data/config/playlists")
DEFINE_DIRECTORY_RESOURCE(SONG_DIRECTORY, "../afterworld/data/config/ball/songs", SONG_TYPE)
DEFINE_DIRECTORY_RESOURCE(GUN_CONFIG_DIRECTORY, "../afterworld/data/config/fps/guns", WEAPON_TYPE)
DEFINE_DIRECTORY_RESOURCE(TRAIT_CONFIG_DIRECTORY, "../afterworld/data/config/fps/traits")
DEFINE_DIRECTORY_RESOURCE(MATERIAL_CONFIG_DIRECTORY, "../afterworld/data/config/materials", MATERIAL_TYPE)
DEFINE_DIRECTORY_RESOURCE(PARTICLE_CONFIG_DIRECTORY, "../afterworld/data/config/particles")
DEFINE_DIRECTORY_RESOURCE(SOUND_CONFIG_DIRECTORY, "../afterworld/data/sounds")
