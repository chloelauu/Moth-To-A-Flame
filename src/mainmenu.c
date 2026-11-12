/**
 * @file        mainmenu.c
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       The main menu: the game title, a glowing Play button with a
 *              circling moth, and a row of Guide, Controls, Credits and Quit
 *              buttons.
 * @par         Polish (2026): Chloe Lau Rey En. New layout, Play glow and moth
 *              animation, animated buttons, and frees its assets.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game_state.h"
#include "../include/game.h"
#include "../include/mode_select.h"
#include "../include/tutorial.h"
#include "../include/utils.h"
#include "../include/controls.h"
#include "../include/game_complete.h"
#include <math.h>

static CP_Image backgroundImage;
CP_Sound menuMusic = NULL;
CP_Sound buttonClick = NULL;

// Buttons (hover animation state, see text_button_update in utils.c)
static TextButton playButton, guideButton, controlsButton, creditsButton, quitButton;

// Layout
#define PLAY_TEXT_SIZE      120.0f  // big "Play" under the title
#define MENU_ROW_TEXT_SIZE  70.0f   // Guide / Controls / Credits / Quit row
#define MENU_ROW_Y          760.0f  // row height, kept above the ground line in the background (around y = 735)
#define MENU_ROW_SPACING    300.0f  // distance between the centres of the row buttons
#define PLAY_Y              490.0f  // centre of the "Play" text

// "Play" animation: a moth drawn to a flame
#define PLAY_GLOW_RADIUS    140.0f  // radius of the warm glow behind "Play"
#define PLAY_BREATH_TIME    3.0f    // seconds for one slow "breath" of the idle glow
#define PLAY_FLARE_TIME     0.3f    // seconds the glow flares after "Play" is clicked
#define MOTH_SIZE           60.0f   // size of the circling moth (about a row button's height)
#define MOTH_LOOP_SPEED     2.2f    // radians per second around the loop
#define MOTH_FLAP_TIME      0.08f   // seconds per wing frame

static CP_Image mothFrames[2];
static float playTimer = 0.0f;       // drives the breathing glow and the moth's wobble
static float playFlareTimer = 0.0f;  // > 0 while the glow flares after a click
static float mothAngle = 0.0f;       // position of the moth around its loop
static float mothFlapTimer = 0.0f;
static int mothFrame = 0;
static float mothLastX = 0.0f;
static bool mothFacingRight = true;

float mainMenuFadeAlpha = 0.0f;       // Alpha value for the fade (0 = transparent, 255 = opaque)
int mainMenuIsFading = 0;            // 0 = no fade, 1 = fading out, 2 = fading in
int mainMenuNextState = -1;          // Next game state to transition to after the fade

void main_menu_init(void) {
	currentState = STATE_MAIN_MENU;
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	backgroundImage = CP_Image_Load("Assets/Backgrounds/menu_background.jpg");

	menuMusic = CP_Sound_LoadMusic("Assets/Audio/BGM/main_menu_theme.wav");
	buttonClick = CP_Sound_Load("Assets/Audio/UI/button_click.wav");

	CP_Sound_PlayMusic(menuMusic);

	text_button_reset(&playButton);
	text_button_reset(&guideButton);
	text_button_reset(&controlsButton);
	text_button_reset(&creditsButton);
	text_button_reset(&quitButton);

	// "Play" animation
	mothFrames[0] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying1.png");
	mothFrames[1] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying2.png");
	playTimer = 0.0f;
	playFlareTimer = 0.0f;
	mothAngle = 0.0f;
	mothFlapTimer = 0.0f;
	mothFrame = 0;
}

// soft warm glow behind "Play": a few layered ember circles, brighter towards the middle
static void draw_play_glow(float x, float y, float brightness, float radiusScale) {
	CP_Settings_NoStroke();
	for (int i = 0; i < 5; i++) {
		float radius = PLAY_GLOW_RADIUS * radiusScale * (1.0f - 0.18f * i);
		float alpha = 14.0f * (1.0f + 0.3f * i) * brightness;
		if (alpha > 255.0f) alpha = 255.0f;
		CP_Settings_Fill(CP_Color_Create(224, 122, 58, (int)alpha));
		CP_Graphics_DrawCircle(x, y, radius * 2.0f);
	}
}

// the moth fluttering in a loose, wobbly loop around "Play" (inside the glow)
static void update_play_moth(float x, float y, float dt, float hover) {
	mothAngle += MOTH_LOOP_SPEED * dt;

	mothFlapTimer += dt;
	if (mothFlapTimer >= MOTH_FLAP_TIME) {
		mothFlapTimer -= MOTH_FLAP_TIME;
		mothFrame = (mothFrame + 1) % 2;
	}

	// an ellipse whose size drifts a little, plus a small jitter, so the loop never looks mechanical
	float radiusX = 100.0f + 12.0f * sinf(mothAngle * 2.3f + playTimer);
	float radiusY = 50.0f + 10.0f * sinf(mothAngle * 1.7f);
	float mothX = x + radiusX * cosf(mothAngle) + 6.0f * sinf(playTimer * 7.0f);
	float mothY = y - 5.0f + radiusY * sinf(mothAngle) + 5.0f * sinf(playTimer * 5.3f);

	if (mothX > mothLastX + 0.01f) mothFacingRight = true;
	else if (mothX < mothLastX - 0.01f) mothFacingRight = false;
	mothLastX = mothX;

	int alpha = (int)(255.0f * hover); // fades in on hover and out when the mouse leaves
	if (alpha > 0) {
		CP_Settings_ImageMode(CP_POSITION_CENTER);
		CP_Image_Draw(mothFrames[mothFrame], mothX, mothY, mothFacingRight ? MOTH_SIZE : -MOTH_SIZE, MOTH_SIZE, alpha);
	}
}

void main_menu_update(void) {
	/* RESET RECT MODE TO CORNER OTHERWISE FADE TRANSITIONS BETWEEN STATES WILL BUG OUT
	BECAUSE THIS SETTING PERSISTS ACROSS DIFFERENT STATES AS THE GRAPHICS SETTINGS ARE GLOBAL WITHIN THE ENGINE CONTEXT */
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

	// draw bg image
	if (backgroundImage != NULL) {
		CP_Image_Draw(backgroundImage,
			(float)CP_System_GetWindowWidth() / 2,
			(float)CP_System_GetWindowHeight() / 2,
			1600,
			900,
			205);
	}

	// Background color (black screen)
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
	CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	CP_Settings_ImageMode(CP_POSITION_CENTER);

	// Draw the background image
	CP_Image_Draw(backgroundImage,
		(float)CP_System_GetWindowWidth() / 2,
		(float)CP_System_GetWindowHeight() / 2,
		1600,
		900,
		205); // Fully opaque

	float dt = CP_System_GetDt();
	float centerX = (float)CP_System_GetWindowWidth() / 2;

	// Draw main menu text (game title stays bigger than the other screen titles)
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_Set(bodyFont);
	CP_Settings_TextSize(FONT_SIZE_LOGO);
	CP_Font_DrawText("Moth To A Flame", (float)CP_System_GetWindowWidth() / 2, (float)CP_System_GetWindowHeight() / 2 - 250.0f);

	// PLAY: breathing glow, circling moth, then the shared button animation
	playTimer += dt;
	float hover = text_button_hover_amount(&playButton);
	float breath = 0.5f + 0.5f * sinf(playTimer * 2.0f * 3.14159265f / PLAY_BREATH_TIME); // 0..1 over 3 seconds
	float flare = 0.0f;
	if (playFlareTimer > 0.0f) {
		flare = sinf((1.0f - playFlareTimer / PLAY_FLARE_TIME) * 3.14159265f); // rises and falls once
	}
	draw_play_glow(centerX, PLAY_Y, 0.55f + 0.25f * breath + 0.6f * hover + 2.0f * flare, 1.0f + 0.1f * hover + 0.35f * flare);

	// the moth is drawn behind the text on the top half of its loop and in front on the bottom half
	bool mothBehind = sinf(mothAngle) < 0.0f;
	if (mothBehind) update_play_moth(centerX, PLAY_Y, dt, hover);

	if (text_button_update(&playButton, "Play", centerX, PLAY_Y, PLAY_TEXT_SIZE, CP_TEXT_ALIGN_H_CENTER, false)) {
		CP_Sound_Play(buttonClick);
		playFlareTimer = PLAY_FLARE_TIME; // flare first, the fade starts when it ends
	}

	if (!mothBehind) update_play_moth(centerX, PLAY_Y, dt, hover);

	if (playFlareTimer > 0.0f) {
		playFlareTimer -= dt;
		if (playFlareTimer <= 0.0f) {
			playFlareTimer = 0.0f;
			if (mainMenuIsFading == 0) { // unless another button already started a fade
				mainMenuIsFading = 1;        // Start fading out
				mainMenuNextState = STATE_MODE_SELECT; // Set next state to the game state
			}
		}
	}

	// Row of buttons near the bottom, evenly spaced and centred
	float guideX = centerX - 1.5f * MENU_ROW_SPACING;
	float controlsX = centerX - 0.5f * MENU_ROW_SPACING;
	float creditsX = centerX + 0.5f * MENU_ROW_SPACING;
	float quitX = centerX + 1.5f * MENU_ROW_SPACING;

	// "Guide" opens the guide screen (controls.c)
	if (text_button_update(&guideButton, "Guide", guideX, MENU_ROW_Y, MENU_ROW_TEXT_SIZE, CP_TEXT_ALIGN_H_CENTER, false)) {
		CP_Sound_Play(buttonClick);
		mainMenuIsFading = 1;        // Start fading out
		mainMenuNextState = STATE_CONTROLS; // Set next state to the game state
	}

	// "Controls" opens the controls screen (tutorial.c)
	if (text_button_update(&controlsButton, "Controls", controlsX, MENU_ROW_Y, MENU_ROW_TEXT_SIZE, CP_TEXT_ALIGN_H_CENTER, false)) {
		CP_Sound_Play(buttonClick);
		mainMenuIsFading = 1;        // Start fading out
		mainMenuNextState = STATE_TUTORIAL; // Set next state to the tutorial state
	}

	if (text_button_update(&creditsButton, "Credits", creditsX, MENU_ROW_Y, MENU_ROW_TEXT_SIZE, CP_TEXT_ALIGN_H_CENTER, false)) {
		CP_Sound_Play(buttonClick);
		mainMenuIsFading = 1;        // Start fading out
		mainMenuNextState = STATE_GAME_COMPLETED;
	}

	if (text_button_update(&quitButton, "Quit", quitX, MENU_ROW_Y, MENU_ROW_TEXT_SIZE, CP_TEXT_ALIGN_H_CENTER, false)) {
		CP_Sound_Play(buttonClick);
		mainMenuIsFading = 1;        // Start fading out
		mainMenuNextState = -1;
	}

	// Handle fade transitions
	if (mainMenuIsFading == 1) { // Fading out
		mainMenuFadeAlpha += 9.0f; // Adjust speed as needed
		if (mainMenuFadeAlpha >= 255.0f) {
			mainMenuFadeAlpha = 255.0f;
			if (mainMenuNextState == -1) {
				CP_Engine_Terminate(); // Quit the game
			}
			else {
				switch (mainMenuNextState) {
				case STATE_TUTORIAL:
					CP_Engine_SetNextGameState(tutorial_init, tutorial_update, tutorial_exit);
					break;
				case STATE_CONTROLS:
					CP_Engine_SetNextGameState(controls_init, controls_update, controls_exit);
					break;
				case STATE_GAME_COMPLETED:
					CP_Engine_SetNextGameState(game_complete_init, game_complete_update, game_complete_exit);
					break;
				case STATE_MODE_SELECT:
					CP_Engine_SetNextGameState(mode_select_init, mode_select_update, mode_select_exit);
					break;
				}
				mainMenuIsFading = 2;  // Start fading in
			}
		}
	}
	else if (mainMenuIsFading == 2) { // Fading in
		mainMenuFadeAlpha -= 9.0f; // Adjust speed as needed
		if (mainMenuFadeAlpha <= 0.0f) {
			mainMenuFadeAlpha = 0.0f;
			mainMenuIsFading = 0; // End fading
		}
	}

	// Draw the fade effect
	if (mainMenuIsFading != 0) {
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)mainMenuFadeAlpha));
		CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	}
}

void main_menu_exit(void) {
	CP_Sound_Free(&menuMusic);
	CP_Sound_Free(&buttonClick);
	CP_Image_Free(&backgroundImage);
	CP_Image_Free(&mothFrames[0]);
	CP_Image_Free(&mothFrames[1]);
}
