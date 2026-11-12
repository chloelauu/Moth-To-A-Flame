/**
 * @file        game.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En: game state management
 *              Tan Shi Qi Charity: drawing of map assets
 * @brief       The main gameplay state: loads and draws the level, runs every
 *              update in order, handles the pause menu and switches between
 *              gameplay, game over and the credits.
 * @par         Polish (2026): Chloe Lau Rey En. Pause menu redesign (fade-in,
 *              spaced flickering title, animated buttons), restarts and screen
 *              changes go through the engine, assets freed once, Debug-only
 *              F1-F5 shortcuts.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/platform.h"
#include "../include/hazards.h"
#include "../include/food.h"
#include "../include/vision_system.h"
#include "../include/light_source.h"
#include "../include/mainmenu.h"
#include "../include/tutorial.h"
#include "../include/game_state.h"
#include "../include/mode_select.h"
#include "../include/game_complete.h"
#include "../include/gameover.h"
#include <stdio.h>

// World size constants
#define WORLD_WIDTH 1600.0f
#define WORLD_UNITS 3
#define TOTAL_WORLD_WIDTH (WORLD_WIDTH * WORLD_UNITS)

CP_Image GroundTile, Background, Clouds, Trees, Platforms;
CP_Color staminaBarColor;
CP_Sound gameMusic = NULL;

// for death message
char deathMessage[256];
DeathCause playerDeathCause;
static float fadeAlpha = 0.0f;
static int isFading = 0;
static int nextState = -1;

// for ground
Hitbox groundHitboxUpper;
float groundTop = 690.0f;
float upperGroundTop = 730.0f;
float treeBaseLevel = 500.0f;

// for stamina bar
float staminaBarX, staminaBarY, staminaBarWidth, staminaBarHeight;
float staminaTip1X, staminaTip1Y, staminaTip2X, staminaTip2Y, staminaTip3X, staminaTip3Y;
float currentStamina = 100.0f;
float maxStamina = 100.0f;

// game state
GameState currentState = STATE_MAIN_MENU;
bool isPaused = false;
bool gameRestarted = false;
static GameState previousState = STATE_GAME; // to notice when the game has just been won (see game_update)

// Add frame limiting
static float lastFrameTime = 0.0f;
#define MIN_FRAME_TIME (1.0f / 60.0f)

// pause menu buttons (hover animation state, see utils.c)
static TextButton resumeButton, pauseMenuButton, pauseQuitButton;
static bool resumeRequested = false; // set when "Resume Game" is clicked, handled like pressing ESC
static bool audioHandled = false;    // true while the music and sound effects are paused by the pause menu

// pause screen look
#define PAUSE_OVERLAY_ALPHA     166     // black at ~65% over the game
#define PAUSE_FADE_TIME         0.2f    // seconds for the pause menu to fade in
#define PAUSE_TITLE_SIZE        160.0f
#define PAUSE_TITLE_SPACING     0.13f   // extra space between the title's letters (fraction of its size)
static float pauseFadeTimer = 0.0f;     // counts up from 0 when the game is paused
static TextButton pauseTitleFlicker;    // slow candle flicker of the "Game Paused" title

void static renderPauseScreen(void) {
	// Draw the vision system overlay first (the square cutout)
	if (selectedMode == GAME_MODE_WITH_VISION) {
		vision_system_draw();
	}

	// fade the whole pause menu in (the game itself is paused, but the frame time still runs)
	pauseFadeTimer += CP_System_GetDt();
	float fade = pauseFadeTimer / PAUSE_FADE_TIME;
	if (fade > 1.0f) fade = 1.0f;

	// dim the paused game
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)(PAUSE_OVERLAY_ALPHA * fade)));
	CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());

	// "Game Paused": big, letter-spaced, with a dark shadow and a slow candle flicker
	float titleX = (float)CP_System_GetWindowWidth() / 2;
	float titleY = (float)CP_System_GetWindowHeight() / 2 - 200;
	float brightness = title_flicker_update(&pauseTitleFlicker);
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)(153 * fade))); // same shadow as the buttons: 3px, black at 60%
	draw_spaced_text(titleFont, "Game Paused", titleX + 3.0f, titleY + 3.0f, PAUSE_TITLE_SIZE, PAUSE_TITLE_SPACING);
	CP_Settings_Fill(CP_Color_Create((int)(200 * brightness), 0, 0, (int)(255 * fade)));
	draw_spaced_text(titleFont, "Game Paused", titleX, titleY, PAUSE_TITLE_SIZE, PAUSE_TITLE_SPACING);

	text_button_set_alpha(fade); // buttons fade in too, but can be clicked straight away

	// buttons can be clicked or used with their key
	float buttonX = (float)CP_System_GetWindowWidth() / 2;
	bool resumeClicked = text_button_update(&resumeButton, "Resume Game (ESC)", buttonX, (float)CP_System_GetWindowHeight() / 2, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
	bool menuClicked = text_button_update(&pauseMenuButton, "Return to Main Menu (M)", buttonX, (float)CP_System_GetWindowHeight() / 2 + 100, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
	bool quitClicked = text_button_update(&pauseQuitButton, "Quit Game (Q)", buttonX, (float)CP_System_GetWindowHeight() / 2 + 200, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
	text_button_set_alpha(1.0f);

	if (resumeClicked) {
		resumeRequested = true; // unpaused at the start of the next update, the same way as ESC
	}

	if (CP_Input_KeyTriggered(KEY_M) || menuClicked) {
		CP_Sound_ResumeGroup(CP_SOUND_GROUP_MUSIC);
		CP_Sound_ResumeGroup(CP_SOUND_GROUP_SFX);

		isPaused = false;  // Unpause first
		isFading = 1;      // Start fade out
		nextState = STATE_MAIN_MENU;
		CP_Engine_SetNextGameState(main_menu_init, main_menu_update, main_menu_exit);
		return;
	}
	else if (CP_Input_KeyTriggered(KEY_Q) || quitClicked) {
		CP_Engine_Terminate();
	}
}


void game_init(void)
{
	// stop anything still playing (game over / win sound, menu music) before the game music starts
	CP_Sound_StopAll();
	gameMusic = CP_Sound_LoadMusic("Assets/Audio/BGM/game_music_theme.wav");
	CP_Sound_PlayMusic(gameMusic);

	currentState = STATE_GAME;
	previousState = STATE_GAME;

	isPaused = false;
	resumeRequested = false;
	audioHandled = false;
	text_button_reset(&resumeButton);
	text_button_reset(&pauseMenuButton);
	text_button_reset(&pauseQuitButton);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

	// load environment assets
	GroundTile = CP_Image_Load("Assets/Backgrounds/Ground Tile.png");
	Background = CP_Image_Load("Assets/Backgrounds/Background.png");
	Clouds = CP_Image_Load("Assets/Backgrounds/Cloud.png");
	Trees = CP_Image_Load("Assets/Backgrounds/Tree.png");
	Platforms = CP_Image_Load("Assets/Backgrounds/platform.png");

	// window size
	// keep the current window mode (fullscreen / windowed) instead of forcing a 1600 x 900 window
	window_mode_apply();
	float windowWidth = (float)CP_System_GetWindowWidth();

	// initialize the upper ground hitbox
	groundHitboxUpper.x = 0.0f;
	groundHitboxUpper.y = upperGroundTop;
	groundHitboxUpper.width = (float)CP_System_GetWindowWidth();
	groundHitboxUpper.height = groundTop - upperGroundTop;  // Height is the difference between ground levels
	groundHitboxUpper.type = RECTANGLE;

	CP_System_SetFrameRate(60.0f);

	// initialize game elements
	frog_init(650.0f, 725.0f); // Set initial position for the frog
	bird_init(patrolStartX);
	player_init();
	platform_init();
	food_init();
	hazard_init();
	lightsource_init();

	// initialize vision system for zoom and tracking of player
	vision_system_init();
}


void game_logic_update(void) {
	if (selectedMode == GAME_MODE_WITH_VISION) {
		vision_system_draw();
	}


	if (CP_Input_KeyTriggered(KEY_ESCAPE) || resumeRequested) {
		resumeRequested = false;
		isPaused = !isPaused;

		if (isPaused) {
			// start the pause menu fading in, with fresh buttons (no half-finished hover/click from last time)
			pauseFadeTimer = 0.0f;
			title_flicker_reset(&pauseTitleFlicker);
			text_button_reset(&resumeButton);
			text_button_reset(&pauseMenuButton);
			text_button_reset(&pauseQuitButton);
		}

		if (isPaused && !audioHandled) {
			CP_Sound_PauseGroup(CP_SOUND_GROUP_MUSIC);
			CP_Sound_PauseGroup(CP_SOUND_GROUP_SFX);
			audioHandled = true;
		}
		else if (!isPaused && audioHandled) {
			CP_Sound_ResumeGroup(CP_SOUND_GROUP_MUSIC);
			CP_Sound_ResumeGroup(CP_SOUND_GROUP_SFX);
			audioHandled = false;
		}
	}

	// Vision system updates
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		visionSystem.isGameOver = false; // Ensure no game-over by vision
	}
	else {
		if (visionSystem.isGameOver) {
			currentState = STATE_GAME_OVER;
			return;
		}
		vision_system_update();
		vision_system_set_zoom(4.0f);
	}

	float windowWidth = (float)CP_System_GetWindowWidth();
	float windowHeight = (float)CP_System_GetWindowHeight();
	float visibleWidth = windowWidth / visionSystem.zoomFactor;
	float visibleHeight = windowHeight / visionSystem.zoomFactor;

	// Calculate background dimensions
	float bgHeight = 900.0f;
	float bgWidth = bgHeight * (1600.0f / 900.0f);

	// Clamp player position
	float minX = windowWidth / 130.0f;
	float maxX = bgWidth - (windowWidth / 45.0f);
	player.x = (player.x < minX) ? minX : player.x;
	player.x = (player.x > maxX) ? maxX : player.x;

	// Clear background
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

	// Begin vision system transform
	vision_system_begin();

	// Calculate visible range
	float leftBound = player.x - (visibleWidth * 0.85f);
	float rightBound = player.x + (visibleWidth * 0.85f);

	// Calculate tile ranges
	float tileWidth = (float)CP_Image_GetWidth(GroundTile);
	float tileHeight = (float)CP_Image_GetHeight(GroundTile);
	int startSection = (int)floorf(leftBound / WORLD_WIDTH);
	int endSection = (int)ceilf(rightBound / WORLD_WIDTH);
	endSection = (endSection >= WORLD_UNITS) ? WORLD_UNITS - 1 : endSection;

	// Add ground tiles range calculation
	float startX = floorf(leftBound / tileWidth) * tileWidth;
	float endX = ceilf(rightBound / tileWidth) * tileWidth;
	if (startX < 0) startX = 0;
	if (endX > TOTAL_WORLD_WIDTH) endX = TOTAL_WORLD_WIDTH;

	float currentElapsedTime = CP_System_GetDt();

	// Set rendering state
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_NoStroke();

	// Draw backgrounds maintaining aspect ratios
	CP_Image_Draw(Background, 0.0f, 0.0f, bgWidth, bgHeight, 255);

	float cloudHeight = 250.0f;
	float cloudWidth = cloudHeight * (1550.0f / 250.0f);
	CP_Image_Draw(Clouds, 0.0f, 0.0f, cloudWidth, cloudHeight, 255);

	float treeHeight = 650.0f;
	float treeWidth = treeHeight * (1650.0f / 650.0f);
	CP_Image_Draw(Trees, 0.0f, 210.0f, treeWidth, treeHeight, 255);

	// Draw ground tiles
	if (startX < 0) startX = 0;
	if (endX > bgWidth) endX = bgWidth;

	for (float x = startX; x < endX; x += tileWidth) {
		if (x >= 0 && x < bgWidth) {
			CP_Image_Draw(GroundTile, x - 0.5f, groundTop, tileWidth + 1.0f, tileHeight, 255);
		}
	}

	// Draw game elements
	platform_draw();
	hazard_draw();
	mate_draw();
	food_draw();
	lightsource_draw();
	frog_draw();
	bird_draw();
	player_draw();

	// End vision system transform
	vision_system_end();

	// Draw overlay in screen space
	vision_system_draw();

	if (isPaused) {
		if (selectedMode == GAME_MODE_WITH_VISION) {
			vision_system_draw();
		}

		renderPauseScreen();
		return;
	}

#ifdef _DEBUG
	// DEVELOPER SHORTCUTS (Debug builds only, during gameplay and not while paused)
	if (CP_Input_KeyTriggered(KEY_F1)) {
		// instant win: put the moth on the mate so the real win code runs (sound, credits)
		player.x = mate.x;
		player.y = mate.y;
		player.hitbox.x = player.x;
		player.hitbox.y = player.y;
		mate_player_collision();
		return;
	}
	if (CP_Input_KeyTriggered(KEY_F2)) {
		// teleport next to the mate (the moth falls if you don't fly)
		player.x = mate.x - 40.0f;
		player.y = mate.y;
		player.hitbox.x = player.x;
		player.hitbox.y = player.y;
	}
	if (CP_Input_KeyTriggered(KEY_F3)) {
		// refill health and light
		player.health = PLAYER_MAX_HEALTH;
		visionSystem.currentRadius = MAX_RADIUS;
	}
	if (CP_Input_KeyTriggered(KEY_F4)) {
		// game over by hazards: player_update below sees no health left and runs the normal death ("Lost to the Wilds")
		player.health = 0.0f;
	}
	if (CP_Input_KeyTriggered(KEY_F5)) {
		// game over by darkness: the same steps vision_system_update takes when the light runs out ("The Light Has Left You..")
		visionSystem.currentRadius = MIN_RADIUS;
		visionSystem.isGameOver = true;
		playerDeathCause = DEATH_BY_DARKNESS;
		currentState = STATE_GAME_OVER;
		return;
	}
#endif

	// Game logic updates (only if not paused)
	// HUD text (in-game messages) uses the body font
	CP_Font_Set(bodyFont);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	drawStaminaBar();
	food_update();
	frog_update(currentElapsedTime);
	player_update();
	bird_update(patrolStartX, patrolEndX);
	lightsource_update();
}

void game_update(void) {
	// remember which state actually ran this frame: the win / death happens in the middle of a gameplay
	// frame, so comparing with currentState at the end of the frame would miss the moment it changed
	GameState stateThisFrame = currentState;

	switch (stateThisFrame) {
	case STATE_MAIN_MENU:
		main_menu_update();
		break;
	case STATE_MODE_SELECT:
		mode_select_update();
		break;
	case STATE_TUTORIAL:
		tutorial_update();
		break;
	case STATE_GAME:
		game_logic_update();
		gameRestarted = false;
		break;
	case STATE_GAME_OVER:
		if (previousState != STATE_GAME_OVER) {
			game_over_init(); // just died: start the game over background animation from the beginning
		}
		// assets stay loaded until this state is left (game_exit runs once, when the engine changes state)
		gameRestarted = true;
		game_over_update();
		break;
	case STATE_GAME_COMPLETED:
		if (previousState != STATE_GAME_COMPLETED) {
			game_complete_init(); // just won: start the credits from the top
		}
		gameRestarted = true;
		game_complete_update();
		break;
	}
	previousState = stateThisFrame;
	//printf("State after key press: %d\n", currentState);
}

void game_exit(void) {
	// free assets
	CP_Image_Free(&GroundTile);
	CP_Image_Free(&Background);
	CP_Image_Free(&Clouds);
	CP_Image_Free(&Trees);
	CP_Image_Free(&Platforms);

	// free other game elements
	player_exit();
	bird_exit();
	frog_exit();
	food_exit();
	hazard_exit();
	lightsource_exit();

	CP_Sound_Free(&gameMusic);

	// free the credits / game over backgrounds if they were shown, and stop the credits scrolling
	game_complete_exit();
	game_over_exit();
}