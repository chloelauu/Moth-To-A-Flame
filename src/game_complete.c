/**
 * @file        game_complete.c
 * @project     Moth To A Flame
 * @author      Tan Shi Qi Charity
 * @brief       The credits screen shown after winning or from the main menu:
 *              scrolling credits over an animated background, then replay, main
 *              menu or quit.
 * @par         Polish (2026): Chloe Lau Rey En. Animated background (fireflies,
 *              moth and mate), animated buttons and back prompt, credits always
 *              restart from the top, replay works from both routes.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game_state.h"
#include "../include/game_complete.h"
#include "../include/mainmenu.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/mode_select.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


float textY = 600; // starts below the screen height
float scrollSpeed = 4.5f;
int creditsFinished = 0;
static TextButton replayButton, menuButton, quitButton, backButton; // hover animation state (see utils.c)
static bool creditsFromGameplay = false; // true after winning, false when opened from the main menu

// ---------------------------------------------------------------------------
// BACKGROUND: menu background + dark overlay, drifting fireflies, and the moth with its mate
#define FIREFLY_COUNT       25
#define CREDITS_OVERLAY     179     // black at ~70% alpha over the background so the text stays readable
#define CREDITS_MOTH_SIZE   50.0f
#define CREDITS_MOTHS_X     1450.0f // bottom-right corner, clear of the centred text, buttons and "< Back"
#define CREDITS_MOTHS_Y     820.0f
#define CREDITS_FLAP_TIME   0.1f    // seconds per wing frame

typedef struct {
	float x, y;
	float riseSpeed;    // pixels per second upwards
	float wobblePhase, wobbleSpeed, wobbleAmount;
	float fadePhase, fadeSpeed;
	float size;
} Firefly;

static CP_Image creditsBackground = NULL;
static CP_Image creditsMothFrames[2] = { NULL, NULL };
static CP_Image creditsMateFrames[2] = { NULL, NULL };
static Firefly fireflies[FIREFLY_COUNT];
static float creditsTimer = 0.0f;
static float creditsFlapTimer = 0.0f;
static int creditsFlapFrame = 0;

static float credits_random(float min, float max) {
	return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

static void firefly_respawn(Firefly* firefly, float y) {
	firefly->x = credits_random(0.0f, (float)CP_System_GetWindowWidth());
	firefly->y = y;
	firefly->riseSpeed = credits_random(12.0f, 30.0f);
	firefly->wobblePhase = credits_random(0.0f, 6.28f);
	firefly->wobbleSpeed = credits_random(0.6f, 1.4f);
	firefly->wobbleAmount = credits_random(8.0f, 20.0f);
	firefly->fadePhase = credits_random(0.0f, 6.28f);
	firefly->fadeSpeed = credits_random(0.5f, 1.3f);
	firefly->size = credits_random(4.0f, 7.0f);
}

// loads the background images the first time the credits are shown
// (the credits can be reached from the main menu or straight from gameplay, where game_complete_init is not called)
static void credits_background_load(void) {
	if (creditsBackground != NULL) {
		return;
	}
	creditsBackground = CP_Image_Load("Assets/Backgrounds/menu_background.jpg");
	creditsMothFrames[0] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying1.png");
	creditsMothFrames[1] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying2.png");
	creditsMateFrames[0] = CP_Image_Load("Assets/Sprites/mate_flying/mate_flying1.png");
	creditsMateFrames[1] = CP_Image_Load("Assets/Sprites/mate_flying/mate_flying2.png");

	for (int i = 0; i < FIREFLY_COUNT; i++) {
		firefly_respawn(&fireflies[i], credits_random(0.0f, (float)CP_System_GetWindowHeight())); // start spread over the screen
	}
	creditsTimer = 0.0f;
}

static void credits_background_draw(void) {
	float dt = CP_System_GetDt();
	float windowWidth = (float)CP_System_GetWindowWidth();
	float windowHeight = (float)CP_System_GetWindowHeight();
	creditsTimer += dt;

	// menu background with a dark overlay
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Image_Draw(creditsBackground, windowWidth / 2, windowHeight / 2, 1600.0f, 900.0f, 255);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, CREDITS_OVERLAY));
	CP_Graphics_DrawRect(0, 0, windowWidth, windowHeight);

	// fireflies: drift up with a slight side-to-side wobble, each fading in and out at its own pace
	for (int i = 0; i < FIREFLY_COUNT; i++) {
		Firefly* firefly = &fireflies[i];
		firefly->y -= firefly->riseSpeed * dt;
		firefly->wobblePhase += firefly->wobbleSpeed * dt;
		firefly->fadePhase += firefly->fadeSpeed * dt;
		if (firefly->y < -20.0f) {
			firefly_respawn(firefly, windowHeight + 20.0f); // wrap back to the bottom
		}

		float x = firefly->x + firefly->wobbleAmount * sinf(firefly->wobblePhase);
		float glow = 0.5f + 0.5f * sinf(firefly->fadePhase); // 0..1
		CP_Settings_Fill(CP_Color_Create(255, 190, 90, (int)(45.0f * glow)));   // faint bigger glow
		CP_Graphics_DrawCircle(x, firefly->y, firefly->size * 4.0f);
		CP_Settings_Fill(CP_Color_Create(255, 215, 120, (int)(170.0f * glow))); // soft core
		CP_Graphics_DrawCircle(x, firefly->y, firefly->size);
	}

	// the moth and its mate fluttering side by side, bobbing gently
	creditsFlapTimer += dt;
	if (creditsFlapTimer >= CREDITS_FLAP_TIME) {
		creditsFlapTimer -= CREDITS_FLAP_TIME;
		creditsFlapFrame = (creditsFlapFrame + 1) % 2;
	}
	float mothBob = 6.0f * sinf(creditsTimer * 1.8f);
	float mateBob = 6.0f * sinf(creditsTimer * 1.8f + 1.2f); // slightly out of step
	// blue moth on the left facing right, pink mate on the right facing left (flipped, as in the game)
	CP_Image_Draw(creditsMothFrames[creditsFlapFrame], CREDITS_MOTHS_X - 30.0f, CREDITS_MOTHS_Y + mothBob, CREDITS_MOTH_SIZE, CREDITS_MOTH_SIZE, 255);
	CP_Image_Draw(creditsMateFrames[(creditsFlapFrame + 1) % 2], CREDITS_MOTHS_X + 30.0f, CREDITS_MOTHS_Y + mateBob, -CREDITS_MOTH_SIZE, CREDITS_MOTH_SIZE, 255);
}

// called by the engine when the credits are opened from the main menu,
// and by game_update when the game has just been won
// puts the credits back to the very beginning
static void credits_reset(void) {
	textY = 600;          // Reset text position
	creditsFinished = 0;  // Ensure scrolling starts
	creditsTimer = 0.0f;
	creditsFlapTimer = 0.0f;
	creditsFlapFrame = 0;
	text_button_reset(&replayButton);
	text_button_reset(&menuButton);
	text_button_reset(&quitButton);
	text_button_reset(&backButton);
}

// every way into the credits comes through here:
// - the main menu's Credits button (the engine calls it)
// - winning, including F1 / F2 in Debug builds (game_update calls it on the first credits frame)
void game_complete_init() {
	credits_reset();
	creditsFromGameplay = (currentState == STATE_GAME_COMPLETED);
}

void game_complete_update() {
	// Background color (black screen)
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
	CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());

	// Animated background behind the credits text
	credits_background_load();
	credits_background_draw();

	// Set text size and color
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

	bool replayClicked = false, menuClicked = false, quitClicked = false;
	if (!creditsFinished) {
		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_TITLE);
		CP_Font_DrawText("Thanks for playing Moth To A Flame!", (float)CP_System_GetWindowWidth() / 2, textY - 50.0f);

		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("Done By", (float)CP_System_GetWindowWidth() / 2, textY + 150.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("TEAM SKIBIDI", (float)CP_System_GetWindowWidth() / 2, textY + 250.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("Code", (float)CP_System_GetWindowWidth() / 2, textY + 450.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("CHLOE LAU         CHARITY TAN", (float)CP_System_GetWindowWidth() / 2, textY + 550.0f);
		CP_Font_DrawText("TRISH TAN         CHOI MENG YEW", (float)CP_System_GetWindowWidth() / 2, textY + 650.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("Art", (float)CP_System_GetWindowWidth() / 2, textY + 850.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("CHARITY TAN       TRISH TAN", (float)CP_System_GetWindowWidth() / 2, textY + 950.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("Created at", (float)CP_System_GetWindowWidth() / 2, textY + 1150.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("DigiPen Institute of Technology Singapore", (float)CP_System_GetWindowWidth() / 2, textY + 1250.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("President", (float)CP_System_GetWindowWidth() / 2, textY + 1450.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("CLAUDE COMAIR", (float)CP_System_GetWindowWidth() / 2, textY + 1550.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_HEADING);
		CP_Font_DrawText("Executives", (float)CP_System_GetWindowWidth() / 2, textY + 1750.0f);

		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("JASON CHU         SAMIR ABOU SAMRA        MICHELE COMAIR", (float)CP_System_GetWindowWidth() / 2, textY + 1850.0f);
		CP_Font_DrawText("ANGELA KUGLER     ERIK MOHRMANN", (float)CP_System_GetWindowWidth() / 2, textY + 1950.0f);
		CP_Font_DrawText("BENJAMIN ELLINGER     MELVIN GONSALVEZ", (float)CP_System_GetWindowWidth() / 2, textY + 2050.0f);

		// move text upwards
		textY -= scrollSpeed;

		// check if last line is out of screen
		if (textY + 2060 < 0) {
			creditsFinished = 1;

			// buttons appear now, start them fresh
			text_button_reset(&replayButton);
			text_button_reset(&menuButton);
			text_button_reset(&quitButton);
		}
	}
	else {
		// buttons can be clicked or used with their key
		float buttonX = (float)CP_System_GetWindowWidth() / 2;
		replayClicked = text_button_update(&replayButton, "Replay (R)", buttonX, (float)(CP_System_GetWindowHeight() / 2.5), FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
		menuClicked = text_button_update(&menuButton, "Return to Main Menu (M)", buttonX, (float)(CP_System_GetWindowHeight() / 2.5 + 100), FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
		quitClicked = text_button_update(&quitButton, "Quit Game (Q)", buttonX, (float)(CP_System_GetWindowHeight() / 2.5 + 200), FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);

		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Font_DrawText("WWW.DIGIPEN.EDU", (float)CP_System_GetWindowWidth() / 2, (float)(CP_System_GetWindowHeight() / 2.5 + 450));
		CP_Font_DrawText("All content (c) 2024 DigiPen Institute of Technology Singapore. All Rights Reserved", (float)CP_System_GetWindowWidth() / 2, (float)(CP_System_GetWindowHeight() / 2.5 + 500));
	}


	// "< Back" in the bottom-left corner, shown during the scroll and after it (same as pressing M)
	bool backClicked = back_button_update(&backButton);

	// Handle input for restarting, going to the main menu, or quitting
	if (CP_Input_KeyTriggered(KEY_R) || replayClicked) {
		//printf("R key pressed: Restarting game\n");

		if (creditsFromGameplay) {
			// after winning: play again in the same mode (the engine runs game_exit, then game_init)
			CP_Engine_SetNextGameStateForced(game_init, game_update, game_exit);
		}
		else {
			// opened from the main menu: no game to replay yet, so choose a mode first
			CP_Engine_SetNextGameState(mode_select_init, mode_select_update, mode_select_exit);
		}
	}
	if (CP_Input_KeyTriggered(KEY_M) || menuClicked || backClicked) {
		// Ensure the main menu is being initialized
		printf("M key pressed. Transitioning to Main Menu...\n");
		CP_Engine_SetNextGameState(main_menu_init, main_menu_update, main_menu_exit);  // Initialize the main menu resources

		printf("Main Menu Initialized.\n");
	}
	if (CP_Input_KeyTriggered(KEY_Q) || quitClicked) {
		CP_Engine_Terminate();  // Quit the game
	}
}


void game_complete_exit() {
	//CP_Image_Free(&copyrightImg);

	// stop the scrolling so nothing carries over to the next time the credits are shown
	credits_reset();

	// free the background images (they are loaded again the next time the credits are shown)
	// (also called from game_exit, so skip it if the credits were never shown)
	if (creditsBackground == NULL) {
		return;
	}
	CP_Image_Free(&creditsBackground);
	CP_Image_Free(&creditsMothFrames[0]);
	CP_Image_Free(&creditsMothFrames[1]);
	CP_Image_Free(&creditsMateFrames[0]);
	CP_Image_Free(&creditsMateFrames[1]);
	creditsBackground = NULL; // makes credits_background_load load them again next time
}