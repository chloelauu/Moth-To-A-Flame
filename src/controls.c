/**
 * @file        controls.c
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       The Guide screen (How To Play): shows the guide picture and
 *              returns to the main menu with the back prompt.
 * @par         Polish (2026): Chloe Lau Rey En. Uses the shared back prompt
 *              (click or Esc) and fonts; no longer frees its assets twice.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "../include/controls.h"
#include "../include/mainmenu.h"
#include "../include/game_state.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include "cprocessing.h"

CP_Image tutorialBackground = NULL;
CP_Sound controlMusic;

static TextButton backButton; // hover animation state (see utils.c)

int width, height;

void controls_init(void) {
	width = CP_System_GetWindowWidth();
	height = CP_System_GetWindowHeight();
	tutorialBackground = CP_Image_Load("Assets/Backgrounds/tutorial.png");
	controlMusic = CP_Sound_LoadMusic("Assets/Audio/BGM/main_menu_theme.wav");

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	text_button_reset(&backButton);

	CP_Sound_PlayMusic(controlMusic);
}

void controls_update(void) {
	CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));


	int width = CP_System_GetWindowWidth();
	int height = CP_System_GetWindowHeight();

	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Image_Draw(tutorialBackground, (float)width / 2, (float)height / 2, 1600.0f, 900.0f, 255);

	// Exit button (bottom-left, see back_button_update in utils.c)
	if (back_button_update(&backButton)) {
		// (the engine calls controls_exit when the state changes)
		currentState = STATE_MAIN_MENU;
		CP_Engine_SetNextGameStateForced(main_menu_init, main_menu_update, main_menu_exit);
	}
}

void controls_exit(void) {
	if (tutorialBackground != NULL) {
		CP_Image_Free(&tutorialBackground);
		tutorialBackground = NULL;
	}

	CP_Sound_Free(&controlMusic);
}
