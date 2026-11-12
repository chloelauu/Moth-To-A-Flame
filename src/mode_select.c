/**
 * @file        mode_select.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Mode select screen: choose Easy (whole map visible) or Hard
 *              (limited light) by clicking a picture or its name, with a
 *              description of each.
 * @par         Polish (2026): Chloe Lau Rey En. New fonts, animated mode
 *              buttons, back prompt and small drawing fixes.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "../include/game.h"
#include "../include/mainmenu.h"
#include "../include/mode_select.h"
#include "../include/game_state.h"
#include "cprocessing.h"

#define RECT_WIDTH 650.0f
#define RECT_HEIGHT 400.0f
#define SPACING 100.0f

typedef struct {
	float x;
	float y;
	float width;
	float height;
} RectangleMode;

static RectangleMode visionRect, noVisionRect;
static CP_Image backgroundImage, easyImage, hardImage;

static GameMode hoveredMode = GAME_MODE_NONE;
GameMode selectedMode = GAME_MODE_NONE;
CP_Sound modeSelectMusic;

static float fadeAlpha = 0.0f;
static int isFading = 0;
static int nextState = -1;

static TextButton backButton, easyButton, hardButton; // hover animation state (see utils.c)

void mode_select_init(void) {
	// Reset states at initialization
	currentState = STATE_MODE_SELECT;
	fadeAlpha = 0.0f;
	isFading = 0;
	nextState = -1;
	hoveredMode = GAME_MODE_NONE;

	modeSelectMusic = CP_Sound_LoadMusic("Assets/Audio/BGM/main_menu_theme.wav");
	float windowWidth = (float)CP_System_GetWindowWidth();
	float windowHeight = (float)CP_System_GetWindowHeight();

	// calculate position for centered rectangles
	float totalWidth = (RECT_WIDTH * 2) + SPACING;
	float startX = (windowWidth - totalWidth) / 2;
	float centerY = windowHeight / 2;

	// intiailize rects
	visionRect.width = RECT_WIDTH;
	visionRect.height = RECT_HEIGHT;
	visionRect.x = startX + RECT_WIDTH + SPACING;
	visionRect.y = centerY - (RECT_HEIGHT / 2);

	noVisionRect.width = RECT_WIDTH;
	noVisionRect.height = RECT_HEIGHT;
	noVisionRect.x = startX;
	noVisionRect.y = centerY - (RECT_HEIGHT / 2);

	backgroundImage = CP_Image_Load("Assets/Backgrounds/menu_background.jpg");
	easyImage = CP_Image_Load("Assets/Backgrounds/easy_mode.png");
	hardImage = CP_Image_Load("Assets/Backgrounds/hard_mode.png");

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	text_button_reset(&backButton);
	text_button_reset(&easyButton);
	text_button_reset(&hardButton);

	CP_Sound_PlayMusic(modeSelectMusic);
}

void mode_select_update(void) {
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

	if (backgroundImage != NULL) {
		CP_Settings_ImageMode(CP_POSITION_CORNER);
		CP_Image_Draw(backgroundImage, 0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), 255);
	}

	// if not fully faded out
	if (fadeAlpha < 255.0f) {
		CP_Settings_RectMode(CP_POSITION_CORNER);

		// get mouse position
		float mouseX = CP_Input_GetMouseX();
		float mouseY = CP_Input_GetMouseY();

		// mode name buttons sit above their pictures
		float easyLabelX = noVisionRect.x + (noVisionRect.width / 2);
		float hardLabelX = visionRect.x + (visionRect.width / 2);
		float labelY = visionRect.y - 40.0f;

		// check hover state (the picture or its name)
		if (text_hovered("Hard Mode", hardLabelX, labelY, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER)) {
			hoveredMode = GAME_MODE_WITH_VISION;
		}
		else if (text_hovered("Easy Mode", easyLabelX, labelY, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER)) {
			hoveredMode = GAME_MODE_WITHOUT_VISION;
		}
		else if (mouseX >= visionRect.x && mouseX <= visionRect.x + visionRect.width &&
			mouseY >= visionRect.y && mouseY <= visionRect.y + visionRect.height) {
			hoveredMode = GAME_MODE_WITH_VISION;
		}
		else if (mouseX >= noVisionRect.x && mouseX <= noVisionRect.x + noVisionRect.width &&
			mouseY >= noVisionRect.y && mouseY <= noVisionRect.y + noVisionRect.height) {
			hoveredMode = GAME_MODE_WITHOUT_VISION;
		}
		else {
			hoveredMode = GAME_MODE_NONE;
		}

		// draw top text
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
		CP_Font_Set(titleFont);
		CP_Settings_TextSize(FONT_SIZE_TITLE);
		CP_Font_DrawText("Select Game Mode", (float)CP_System_GetWindowWidth() / 2, 100.0f);

		// draw rects
		CP_Settings_NoFill();
		CP_Settings_StrokeWeight(5.0f);

		// colro for highlighting
		CP_Color defaultColor = CP_Color_Create(255, 255, 255, 255);
		CP_Color highlightColor = CP_Color_Create(224, 122, 58, 255); // same ember orange as hovered buttons

		// no vision mode rect
		CP_Settings_Stroke((hoveredMode == GAME_MODE_WITHOUT_VISION) ? highlightColor : defaultColor);
		CP_Graphics_DrawRect(noVisionRect.x, noVisionRect.y, noVisionRect.width, noVisionRect.height);

		// drawing image for easy mode (no vision limit)
		if (easyImage != NULL) {
			CP_Image_Draw(easyImage, noVisionRect.x, noVisionRect.y, noVisionRect.width, noVisionRect.height, 255);
		}

		// vision mode rect
		CP_Settings_Stroke((hoveredMode == GAME_MODE_WITH_VISION) ? highlightColor : defaultColor);
		CP_Graphics_DrawRect(visionRect.x, visionRect.y, visionRect.width, visionRect.height);

		// draw image for hard mode (w vision limit)
		if (hardImage != NULL) {
			CP_Image_Draw(hardImage, visionRect.x, visionRect.y, visionRect.width, visionRect.height, 255);
		}

		// mode name buttons (hovered together with their picture; clicking either selects the mode)
		bool easyChosen = text_button_update(&easyButton, "Easy Mode", easyLabelX, labelY, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER,
			hoveredMode == GAME_MODE_WITHOUT_VISION);
		bool hardChosen = text_button_update(&hardButton, "Hard Mode", hardLabelX, labelY, FONT_SIZE_BUTTON, CP_TEXT_ALIGN_H_CENTER,
			hoveredMode == GAME_MODE_WITH_VISION);

		// Exit button (bottom-left, see back_button_update in utils.c)
		if (back_button_update(&backButton)) {
			// (the engine calls mode_select_exit when the state changes)
			currentState = STATE_MAIN_MENU;
			CP_Engine_SetNextGameStateForced(main_menu_init, main_menu_update, main_menu_exit);
		}

		// draw hover text
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Font_Set(bodyFont);
		CP_Settings_TextSize(FONT_SIZE_BODY);
		if (hoveredMode == GAME_MODE_WITH_VISION) {
			CP_Font_DrawText("Step into a mode where danger is always just around the corner! With limited visibility of",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 50.0f);
			CP_Font_DrawText("environmental hazards, every move requires careful thought and strategy. You will need to plan ahead",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 100.0f);
			CP_Font_DrawText("and stay alert, as the map hides its surprises, and only the sharpest players can outsmart them!",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 150.0f);
		}
		else if (hoveredMode == GAME_MODE_WITHOUT_VISION) {
			CP_Font_DrawText("Dive into a world where nothing hides in the shadows! With full visibility across the map,",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 50.0f);
			CP_Font_DrawText("you can focus entirely on tackling environmental hazards.",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 100.0f);
			CP_Font_DrawText("Surprises still await, but you will always have the advantage of seeing them coming!",
				(float)CP_System_GetWindowWidth() / 2, visionRect.y + visionRect.height + 150.0f);
		}

		// handle selection of mode (once the clicked button's animation has finished)
		if (isFading == 0 && (easyChosen || hardChosen)) {
			selectedMode = easyChosen ? GAME_MODE_WITHOUT_VISION : GAME_MODE_WITH_VISION;
			isFading = 1;  // Start fade out
			currentState = STATE_GAME;
			nextState = STATE_GAME;
			CP_Engine_SetNextGameState(game_init, game_update, game_exit);
		}
	}

	// Handle fade transitions
	if (isFading == 1) { // Fading out
		fadeAlpha += 7.0f;
		if (fadeAlpha >= 255.0f) {
			fadeAlpha = 255.0f;
			if (nextState != -1) {
				currentState = nextState;
			}
			isFading = 2;  // Start fading in
		}
	}
	else if (isFading == 2) { // Fading in
		fadeAlpha -= 7.0f;
		if (fadeAlpha <= 0.0f) {
			fadeAlpha = 0.0f;
			isFading = 0;
		}
	}

	// Draw fade overlay
	if (fadeAlpha > 0) {
		CP_Settings_NoStroke();
		CP_Settings_RectMode(CP_POSITION_CORNER);
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)fadeAlpha));
		CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	}
}

void mode_select_exit(void) {
	if (backgroundImage != NULL) {
		CP_Image_Free(&backgroundImage);
		backgroundImage = NULL;
	}

	if (easyImage != NULL) {
		CP_Image_Free(&easyImage);
	}
	if (hardImage != NULL) {
		CP_Image_Free(&hardImage);
	}

	CP_Sound_Free(&modeSelectMusic);
}