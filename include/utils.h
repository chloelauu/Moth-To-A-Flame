/**
 * @file        utils.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the hitbox struct and collision checks, plus the shared
 *              UI sizes, fonts, text buttons and window mode functions.
 * @par         Polish (2026): Chloe Lau Rey En. Added the shared UI sizes,
 *              fonts, TextButton, back prompt and window mode declarations.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include <stdbool.h>

typedef enum {
	RECTANGLE,
	CIRCLE
} hitboxType;

typedef struct {
	float x, y; // top-left coord for rectangle, center coord for circle
	float width, height; // dimension of rctangle hitboxes
	float radius; // radius of circle
	float diameter;
	hitboxType type;
} Hitbox;

bool checkCollision(Hitbox a, Hitbox b);
bool checkCircleRectangleCollision(Hitbox circle, Hitbox rectangle);

// ---------------------------------------------------------------------------
// shared UI fonts and text sizes (used by every screen)
// titleFont = Darkening        -> big screen titles ONLY (it has no digits, never use it for text with numbers)
// bodyFont  = WickedScaryMovie -> main menu logo, buttons, body text, HUD messages, credits
#include "cprocessing.h"

#define FONT_SIZE_LOGO      200.0f  // main menu "Moth To A Flame" title only (bodyFont)
#define FONT_SIZE_TITLE     120.0f  // screen titles (titleFont)
#define FONT_SIZE_HEADING   64.0f   // section headings in the credits and controls screen (bodyFont)
#define FONT_SIZE_BUTTON    60.0f   // buttons and menu options (bodyFont)
#define FONT_SIZE_BODY      40.0f   // body text and in-game messages (bodyFont)

// "[Esc] Back" prompt, bottom-left corner of the screen, centred in the dark strip below the ground
#define BACK_BUTTON_X           35.0f   // left edge of the "Esc" key box
#define BACK_BUTTON_Y           845.0f  // vertical centre of the prompt
#define BACK_BUTTON_SIZE        44.0f   // size of the word "Back"
#define BACK_KEY_TEXT_SIZE      36.0f   // size of "Esc" inside the key box
#define BACK_KEY_BOX_HEIGHT     40.0f
#define BACK_KEY_BOX_PADDING    16.0f   // space either side of "Esc" in the box
#define BACK_KEY_GAP            14.0f   // space between the key box and "Back"

// animation state of one clickable text button (see text_button_update in utils.c)
typedef struct {
	float hoverT;       // 0 = normal, 1 = fully hovered (eases between the two)
	float clickTimer;   // > 0 while the click shrink is playing
	float flickerWait;  // time until the next candle-flicker dip while hovered
	float dipTimer;     // > 0 while the text is dipped in brightness
	float dipStrength;  // brightness multiplier during the current dip
} TextButton;

extern CP_Font titleFont, bodyFont;

// window: the game is drawn at a fixed 1600 x 900 and starts in a centred window of that size.
// F11 fullscreen switches the monitor to that resolution (CP_System_FullscreenAdvanced), so every hard-coded position stays correct.
#define GAME_WIDTH  1600
#define GAME_HEIGHT 900
void window_mode_apply(void);
void window_mode_update(void);

void fonts_load(void);
float body_text_width(const char* text, float size);
float font_text_width(CP_Font font, const char* text, float size);
void draw_spaced_text(CP_Font font, const char* text, float centerX, float y, float size, float spacing);
void title_flicker_reset(TextButton* state);
float title_flicker_update(TextButton* state);
void text_button_set_alpha(float alpha);
bool text_hovered(const char* text, float x, float y, float size, CP_TEXT_ALIGN_HORIZONTAL align);
void text_button_reset(TextButton* button);
float text_button_hover_amount(const TextButton* button);
bool text_button_update(TextButton* button, const char* text, float x, float y, float size, CP_TEXT_ALIGN_HORIZONTAL align, bool forceHover);
bool back_button_update(TextButton* button);
