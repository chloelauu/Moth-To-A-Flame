/**
 * @file        tutorial.c
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       The Controls screen: key boxes and labels for flying, dashing,
 *              crawling and crawl dashing, which flash when the matching key is
 *              pressed.
 * @par         Polish (2026): Chloe Lau Rey En. Redesigned layout (panel, grid,
 *              clearer labels) on the menu background, back prompt, and frees
 *              its assets.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "../include/mainmenu.h"
#include "../include/game_state.h"
#include "../include/utils.h"
#include "cprocessing.h"
#include <stdio.h>
#include <stdbool.h>

static CP_Image backgroundImage;
CP_Sound tutMusic = NULL;

// Colors
CP_Color flashColor;

// Flash timers (a key box flashes darker while its key is held)
float flashA = 0.0f;
float flashD = 0.0f;
float flashSpace = 0.0f;
float flashLMB = 0.0f;

// Flash duration
const float FLASH_DURATION = 0.05f; // 0.2 seconds

/* EXIT BUTTON PROPERTIES */
static TextButton backButton; // hover animation state (see utils.c)

/* LAYOUT (a 2 x 2 grid inside a dark panel: Flight | Dash on top, Crawl | Crawl dash below) */
#define PANEL_LEFT          100.0f
#define PANEL_RIGHT         1500.0f
#define PANEL_TOP           170.0f  // below the "Controls" title
#define PANEL_BOTTOM        660.0f  // above the dark strip at the bottom of the background
#define PANEL_ALPHA         140     // black at ~55%
#define COLUMN_PADDING      20.0f   // space between a column's content and the panel edge / middle line
#define ROW1_HEADING_Y      225.0f
#define ROW2_HEADING_Y      455.0f
#define KEYS_OFFSET_Y       80.0f   // key row, below its heading
#define LABEL_OFFSET_Y      150.0f  // action label, below its heading

/* KEY BOXES (same yellow rounded style as before) */
#define KEY_TEXT_SIZE       44.0f
#define KEY_BOX_HEIGHT      70.0f   // every key box is the same height
#define KEY_BOX_MIN_WIDTH   70.0f
#define KEY_BOX_PADDING     24.0f   // space either side of the key text
#define KEY_BOX_RADIUS      15.0f
#define COMBO_GAP           12.0f   // space between the parts of a key combination

enum { FLASH_NONE, FLASH_KEY_A, FLASH_KEY_D, FLASH_KEY_SPACE, FLASH_MOUSE_LEFT };

#define MAX_COMBO_PARTS 5

// one part of a key combination: a key box (isKey) or plain text such as "+", "/" or "Aim with the mouse"
typedef struct {
	const char* text;
	bool isKey;
	int flash; // which key's flash timer colours this box
} ComboPart;

// one control: its key combination and a single action label centred under it
typedef struct {
	ComboPart parts[MAX_COMBO_PARTS];
	int partCount;
	const char* label;
} ControlEntry;

// Controls as they work in player.c (player_flight, player_crawl):
static const ControlEntry flightControls[] = {
	{ { { "A", true, FLASH_KEY_A } }, 1, "Fly up-left" },     // each tap is one flap
	{ { { "D", true, FLASH_KEY_D } }, 1, "Fly up-right" },
	{ { { "A", true, FLASH_KEY_A }, { "+", false, FLASH_NONE }, { "D", true, FLASH_KEY_D } }, 3, "Fly straight up" }, // hold both
};
static const ControlEntry dashControls[] = {
	{ { { "A", true, FLASH_KEY_A }, { "/", false, FLASH_NONE }, { "D", true, FLASH_KEY_D }, { "+", false, FLASH_NONE }, { "Space", true, FLASH_KEY_SPACE } }, 5,
	  "Dash (hold Space, then tap A or D)" },
};
static const ControlEntry crawlControls[] = {
	{ { { "Left Click", true, FLASH_MOUSE_LEFT }, { "+", false, FLASH_NONE }, { "Aim with the mouse", false, FLASH_NONE } }, 3, "Crawl toward the cursor" },
};
static const ControlEntry crawlDashControls[] = {
	{ { { "Left Click", true, FLASH_MOUSE_LEFT }, { "+", false, FLASH_NONE }, { "Space", true, FLASH_KEY_SPACE } }, 3, "Crawl faster" },
};

void tutorial_init() {
	// Initialize text settings
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	text_button_reset(&backButton);

	// Initialize music settings
	tutMusic = CP_Sound_LoadMusic("Assets/Audio/BGM/main_menu_theme.wav");
	CP_Sound_PlayMusic(tutMusic);

	// same forest scene as the main menu (controls.png has the old key layout painted into it)
	backgroundImage = CP_Image_Load("Assets/Backgrounds/menu_background.jpg");

	flashColor = CP_Color_Create(145, 116, 28, 204);
}

static bool is_flashing(int flash) {
	switch (flash) {
	case FLASH_KEY_A: return flashA > 0;
	case FLASH_KEY_D: return flashD > 0;
	case FLASH_KEY_SPACE: return flashSpace > 0;
	case FLASH_MOUSE_LEFT: return flashLMB > 0;
	}
	return false;
}

static float combo_part_width(const ComboPart* part) {
	float textWidth = body_text_width(part->text, KEY_TEXT_SIZE);
	if (!part->isKey) {
		return textWidth;
	}
	float boxWidth = textWidth + 2.0f * KEY_BOX_PADDING;
	return (boxWidth < KEY_BOX_MIN_WIDTH) ? KEY_BOX_MIN_WIDTH : boxWidth;
}

static float combo_width(const ControlEntry* entry) {
	float width = 0.0f;
	for (int i = 0; i < entry->partCount; i++) {
		width += combo_part_width(&entry->parts[i]);
	}
	return width + COMBO_GAP * (entry->partCount - 1);
}

// draws a key combination centred on centerX
static void draw_combo(const ControlEntry* entry, float centerX, float y) {
	float x = centerX - combo_width(entry) / 2;

	CP_Font_Set(bodyFont);
	CP_Settings_TextSize(KEY_TEXT_SIZE);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();

	for (int i = 0; i < entry->partCount; i++) {
		const ComboPart* part = &entry->parts[i];
		float width = combo_part_width(part);
		float partCenterX = x + width / 2;

		if (part->isKey) {
			CP_Settings_Fill(is_flashing(part->flash) ? flashColor : CP_Color_Create(241, 194, 50, 255));
			CP_Graphics_DrawRectAdvanced(x, y - KEY_BOX_HEIGHT / 2, width, KEY_BOX_HEIGHT, 0.0f, KEY_BOX_RADIUS);
			CP_Settings_Fill(CP_Color_Create(30, 30, 30, 255)); // key text: dark grey
		}
		else {
			CP_Settings_Fill(CP_Color_Create(217, 212, 204, 255)); // "+", "/" and hints: pale grey
		}
		CP_Font_DrawText(part->text, partCenterX, y);

		x += width + COMBO_GAP;
	}
}

// draws one section (heading + its controls spread evenly across the column)
static void draw_section(const char* heading, const ControlEntry* entries, int count, float left, float right, float headingY) {
	float centerX = (left + right) / 2;

	// heading: ember orange, larger
	CP_Font_Set(bodyFont);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_TextSize(FONT_SIZE_HEADING);
	CP_Settings_Fill(CP_Color_Create(224, 122, 58, 255));
	CP_Font_DrawText(heading, centerX, headingY);

	// each control takes the width of its keys or its label (whichever is wider), with equal gaps between them
	float widths[3] = { 0.0f, 0.0f, 0.0f };
	float totalWidth = 0.0f;
	for (int i = 0; i < count; i++) {
		float keysWidth = combo_width(&entries[i]);
		float labelWidth = body_text_width(entries[i].label, FONT_SIZE_BODY);
		widths[i] = (keysWidth > labelWidth) ? keysWidth : labelWidth;
		totalWidth += widths[i];
	}
	float gap = ((right - left) - totalWidth) / (count + 1);

	float x = left + gap;
	for (int i = 0; i < count; i++) {
		float controlCenterX = x + widths[i] / 2;
		draw_combo(&entries[i], controlCenterX, headingY + KEYS_OFFSET_Y);

		// action label: pale grey, smaller, centred under the whole combination
		CP_Settings_TextSize(FONT_SIZE_BODY);
		CP_Settings_Fill(CP_Color_Create(217, 212, 204, 255));
		CP_Font_DrawText(entries[i].label, controlCenterX, headingY + LABEL_OFFSET_Y);

		x += widths[i] + gap;
	}
}

void tutorial_update() {
	float dt = CP_System_GetDt();

	// Draw the background image
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Image_Draw(backgroundImage,
		(float)CP_System_GetWindowWidth() / 2,
		(float)CP_System_GetWindowHeight() / 2,
		1600,
		900,
		255);

	// Tutorial title text (outside the panel)
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_Set(titleFont);
	CP_Settings_TextSize(FONT_SIZE_TITLE);
	CP_Font_DrawText("Controls", (float)CP_System_GetWindowWidth() / 2, (float)CP_System_GetWindowHeight() / 2 - 350.0f);

	// Dark rounded panel behind the controls, with faint lines between the four sections
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, PANEL_ALPHA));
	CP_Graphics_DrawRectAdvanced(PANEL_LEFT, PANEL_TOP, PANEL_RIGHT - PANEL_LEFT, PANEL_BOTTOM - PANEL_TOP, 0.0f, 20.0f);

	float panelMiddleX = (PANEL_LEFT + PANEL_RIGHT) / 2;
	float rowDividerY = (ROW1_HEADING_Y + LABEL_OFFSET_Y + ROW2_HEADING_Y) / 2;
	CP_Settings_Stroke(CP_Color_Create(217, 212, 204, 40));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawLine(panelMiddleX, PANEL_TOP + 20.0f, panelMiddleX, PANEL_BOTTOM - 20.0f);
	CP_Graphics_DrawLine(PANEL_LEFT + 20.0f, rowDividerY, PANEL_RIGHT - 20.0f, rowDividerY);
	CP_Settings_NoStroke();

	// Check for key presses and set flash timers
	if (CP_Input_KeyDown(KEY_A)) flashA = FLASH_DURATION;
	if (CP_Input_KeyDown(KEY_D)) flashD = FLASH_DURATION;
	if (CP_Input_KeyDown(KEY_SPACE)) flashSpace = FLASH_DURATION;
	if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT)) flashLMB = FLASH_DURATION;

	// Sections
	float leftColumnLeft = PANEL_LEFT + COLUMN_PADDING;
	float leftColumnRight = panelMiddleX - COLUMN_PADDING;
	float rightColumnLeft = panelMiddleX + COLUMN_PADDING;
	float rightColumnRight = PANEL_RIGHT - COLUMN_PADDING;

	draw_section("Flight", flightControls, 3, leftColumnLeft, leftColumnRight, ROW1_HEADING_Y);
	draw_section("Dash", dashControls, 1, rightColumnLeft, rightColumnRight, ROW1_HEADING_Y);
	draw_section("Crawl", crawlControls, 1, leftColumnLeft, leftColumnRight, ROW2_HEADING_Y);
	draw_section("Crawl dash", crawlDashControls, 1, rightColumnLeft, rightColumnRight, ROW2_HEADING_Y);

	// Decrease flash timers
	if (flashA > 0) flashA -= dt;
	if (flashD > 0) flashD -= dt;
	if (flashSpace > 0) flashSpace -= dt;
	if (flashLMB > 0) flashLMB -= dt;

	// Exit button (bottom-left, see back_button_update in utils.c)
	if (back_button_update(&backButton)) {
		CP_Engine_SetNextGameStateForced(main_menu_init, main_menu_update, main_menu_exit);
		currentState = STATE_MAIN_MENU; // Transition to the main menu state
	}
}

void tutorial_exit(void) {
	CP_Sound_Free(&tutMusic);
	CP_Image_Free(&backgroundImage);
}
