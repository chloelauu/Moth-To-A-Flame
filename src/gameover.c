/**
 * @file        gameover.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       The game over screen: shows how the player died over an animated
 *              background and offers restart, main menu or quit.
 * @par         Polish (2026): Chloe Lau Rey En. Animated backgrounds (embers
 *              for hazards, fading light for darkness), animated buttons,
 *              restart and main menu go through the engine.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game_state.h"
#include "../include/game.h"
#include "../include/gameover.h"
#include "../include/mainmenu.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <math.h>

CP_Sound gameOverSound;
bool soundPlayed = false;
static TextButton restartButton, menuButton, quitButton; // hover animation state (see utils.c)

// ---------------------------------------------------------------------------
// BACKGROUND: darkened menu background + an animation that depends on how the player died
#define GAME_OVER_OVERLAY       204     // black at ~80% over the background
#define EMBER_COUNT             15      // "Lost to the Wilds": dying embers
#define RED_PULSE_TIME          3.0f    // seconds for one slow pulse of the red tint
#define LIGHT_FADE_TIME         4.0f    // "The Light Has Left You..": seconds for the light to shrink away
#define LIGHT_START_RADIUS      220.0f

typedef struct {
    float x, y;
    float riseSpeed;    // pixels per second upwards
    float wobblePhase, wobbleSpeed, wobbleAmount;
    float size;
} Ember;

static CP_Image gameOverBackground = NULL;
static Ember embers[EMBER_COUNT];
static float gameOverTimer = 0.0f;

static float game_over_random(float min, float max) {
    return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

static void ember_respawn(Ember* ember, float y) {
    ember->x = game_over_random(0.0f, (float)CP_System_GetWindowWidth());
    ember->y = y;
    ember->riseSpeed = game_over_random(20.0f, 45.0f);
    ember->wobblePhase = game_over_random(0.0f, 6.28f);
    ember->wobbleSpeed = game_over_random(0.8f, 1.6f);
    ember->wobbleAmount = game_over_random(6.0f, 16.0f);
    ember->size = game_over_random(3.0f, 6.0f);
}

// called every time the game over screen is entered (game_update calls it on the first game over frame)
void game_over_init() {
    // fonts are loaded once by fonts_load() (see utils.c)
    if (gameOverBackground == NULL) {
        gameOverBackground = CP_Image_Load("Assets/Backgrounds/menu_background.jpg");
    }

    // restart the background animation
    gameOverTimer = 0.0f;
    for (int i = 0; i < EMBER_COUNT; i++) {
        ember_respawn(&embers[i], game_over_random(300.0f, (float)CP_System_GetWindowHeight())); // start spread over the lower screen
    }
}

static void game_over_background_draw(void) {
    float dt = CP_System_GetDt();
    float windowWidth = (float)CP_System_GetWindowWidth();
    float windowHeight = (float)CP_System_GetWindowHeight();
    gameOverTimer += dt;

    // heavily darkened menu background
    if (gameOverBackground != NULL) {
        CP_Settings_ImageMode(CP_POSITION_CENTER);
        CP_Image_Draw(gameOverBackground, windowWidth / 2, windowHeight / 2, 1600.0f, 900.0f, 255);
    }
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_NoStroke();
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, GAME_OVER_OVERLAY));
    CP_Graphics_DrawRect(0, 0, windowWidth, windowHeight);

    if (playerDeathCause == DEATH_BY_HAZARDS) {
        // very faint red tint, slowly pulsing
        float pulse = 0.5f + 0.5f * sinf(gameOverTimer * 2.0f * 3.14159265f / RED_PULSE_TIME);
        CP_Settings_Fill(CP_Color_Create(120, 0, 0, (int)(10.0f + 18.0f * pulse)));
        CP_Graphics_DrawRect(0, 0, windowWidth, windowHeight);

        // dying embers: drift up with a slight wobble, fading out as they rise, then start again at the bottom
        for (int i = 0; i < EMBER_COUNT; i++) {
            Ember* ember = &embers[i];
            ember->y -= ember->riseSpeed * dt;
            ember->wobblePhase += ember->wobbleSpeed * dt;
            if (ember->y < 250.0f) {
                ember_respawn(ember, windowHeight + 10.0f);
            }

            float life = (ember->y - 250.0f) / (windowHeight - 250.0f); // 1 at the bottom, 0 where it dies out
            if (life < 0.0f) life = 0.0f;
            if (life > 1.0f) life = 1.0f;
            float x = ember->x + ember->wobbleAmount * sinf(ember->wobblePhase);
            CP_Settings_Fill(CP_Color_Create(255, 70, 30, (int)(50.0f * life)));    // faint glow
            CP_Graphics_DrawCircle(x, ember->y, ember->size * 4.0f);
            CP_Settings_Fill(CP_Color_Create(255, 110, 50, (int)(210.0f * life)));  // hot core
            CP_Graphics_DrawCircle(x, ember->y, ember->size);
        }
    }
    else if (playerDeathCause == DEATH_BY_DARKNESS) {
        // the player's light: layered soft circles that shrink and dim over 4 seconds, then it stays dark
        float remaining = 1.0f - gameOverTimer / LIGHT_FADE_TIME;
        if (remaining > 0.0f) {
            float radius = LIGHT_START_RADIUS * remaining;
            for (int i = 0; i < 5; i++) {
                float layerRadius = radius * (1.0f - 0.18f * i);
                float alpha = 16.0f * (1.0f + 0.35f * i) * remaining;
                CP_Settings_Fill(CP_Color_Create(237, 227, 176, (int)alpha)); // same colour as the light glows in the game
                CP_Graphics_DrawCircle(windowWidth / 2, windowHeight / 2, layerRadius * 2.0f);
            }
        }
    }
}

void game_over_update() {

    if (!soundPlayed) {
        CP_Sound_StopAll();
        CP_Sound_Play(gameOverSound);
        soundPlayed = true;

        // first frame of the game over screen: start with fresh buttons
        text_button_reset(&restartButton);
        text_button_reset(&menuButton);
        text_button_reset(&quitButton);
    }

    // Background color (black screen)
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
    CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());

    // Animated background behind the title and buttons
    game_over_background_draw();

    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_Set(titleFont);
    CP_Settings_TextSize(FONT_SIZE_TITLE);
    CP_Settings_Fill(CP_Color_Create(200, 0, 0, 255));
    if (playerDeathCause == DEATH_BY_DARKNESS) {
        CP_Font_DrawText("The Light Has Left You..", (float)CP_System_GetWindowWidth() / 2, (float)(CP_System_GetWindowHeight() / 2 - 200));
    }
    else if (playerDeathCause == DEATH_BY_HAZARDS) {
        CP_Font_DrawText("Lost to the Wilds", (float)CP_System_GetWindowWidth() / 2, (float)(CP_System_GetWindowHeight() / 2 - 200));
    }

    // Instruction texts (buttons can be clicked or used with their key)
    float buttonX = (float)CP_System_GetWindowWidth() / 2;
    bool restartClicked = text_button_update(&restartButton, "Restart (R)", buttonX, (float)CP_System_GetWindowHeight() / 2, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
    bool menuClicked = text_button_update(&menuButton, "Return to Main Menu (M)", buttonX, (float)(CP_System_GetWindowHeight() / 2 + 100), FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);
    bool quitClicked = text_button_update(&quitButton, "Quit Game (Q)", buttonX, (float)(CP_System_GetWindowHeight() / 2 + 200), FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER, false);

    // Handle input for restarting, going to the main menu, or quitting
    // both go through the engine, so game_exit runs once before the next screen starts
    if (CP_Input_KeyTriggered(KEY_R) || restartClicked) {
        CP_Engine_SetNextGameStateForced(game_init, game_update, game_exit);  // Restart the game
    }
    if (CP_Input_KeyTriggered(KEY_M) || menuClicked) {
        CP_Engine_SetNextGameState(main_menu_init, main_menu_update, main_menu_exit);  // Go to the main menu
    }
    if (CP_Input_KeyTriggered(KEY_Q) || quitClicked) {
        CP_Engine_Terminate();  // Quit the game
    }
}

// frees the game over background (called from game_exit; gameOverSound is freed in player_exit)
void game_over_exit() {
    if (gameOverBackground != NULL) {
        CP_Image_Free(&gameOverBackground);
        gameOverBackground = NULL;
    }
}