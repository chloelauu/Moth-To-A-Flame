/**
 * @file        utils.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Shared helpers: hitbox collision checks, the game's two fonts
 *              and text widths, animated text buttons, the "[Esc] Back" prompt,
 *              letter-spaced titles and the windowed/fullscreen toggle.
 * @par         Polish (2026): Chloe Lau Rey En. Shared fonts, text widths,
 *              animated buttons, back prompt, spaced title text, title flicker
 *              and window mode (F11) added; collision checks unchanged.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/utils.h"
#include "math.h"
#include <stdlib.h>

bool checkCollision(Hitbox hitboxA, Hitbox hitboxB) {
	// checks if both hitboxes are circles
	if (hitboxA.type == CIRCLE && hitboxB.type == CIRCLE) {
		float deltaX = hitboxA.x - hitboxB.x;
		float deltaY = hitboxA.y - hitboxB.y;
		float centerDistance = (float)sqrt(deltaX * deltaX + deltaY * deltaY);
		return centerDistance <= (hitboxA.radius + hitboxB.radius);
	}

	// check if both hitboxes are rectangle
	if (hitboxA.type == RECTANGLE && hitboxB.type == RECTANGLE) {
		return !(hitboxA.x > hitboxB.x + hitboxB.width || hitboxA.x + hitboxA.width < hitboxB.x || hitboxA.y > hitboxB.y + hitboxB.height || hitboxA.y + hitboxA.height < hitboxB
			.y);
	}

	// check if one hitbox is circle and other hitbox is rectangle
	if (hitboxA.type == CIRCLE && hitboxB.type == RECTANGLE) {
		return checkCircleRectangleCollision(hitboxA, hitboxB);
	}
	else if (hitboxA.type == RECTANGLE && hitboxB.type == CIRCLE) {
		return checkCircleRectangleCollision(hitboxB, hitboxA);
	}

	return false;
}

bool checkCircleRectangleCollision(Hitbox circle, Hitbox rectangle) {
	// Find the closest point on the rectangle to the circle's center
	float closestPointX = fmaxf(rectangle.x, fminf(circle.x, rectangle.x + rectangle.width));
	float closestPointY = fmaxf(rectangle.y, fminf(circle.y, rectangle.y + rectangle.height));

	// Calculate the distance between the circle's center and this closest point
	float distanceX = circle.x - closestPointX;
	float distanceY = circle.y - closestPointY;

	// Calculate the squared distance to avoid unnecessary sqrt operations
	float distanceSquared = distanceX * distanceX + distanceY * distanceY;
	return distanceSquared <= (circle.radius * circle.radius);
}

int is_button_hovered(float buttonX, float buttonY, float buttonWidth, float buttonHeight) {
	float mouseX = CP_Input_GetMouseX();
	float mouseY = CP_Input_GetMouseY();

	return (mouseX >= buttonX && mouseX <= buttonX + buttonWidth &&
		mouseY >= buttonY && mouseY <= buttonY + buttonHeight);
}


CP_Font titleFont = NULL, bodyFont = NULL;

// loads the shared fonts once (CProcessing has no way to free fonts, so never load them again)
void fonts_load(void) {
	if (titleFont == NULL) {
		titleFont = CP_Font_Load("Assets/Fonts/Darkening.otf");
	}
	if (bodyFont == NULL) {
		bodyFont = CP_Font_Load("Assets/Fonts/WickedScaryMovie.ttf");
	}
}

// ---------------------------------------------------------------------------
// WINDOW MODE (windowed 1600 x 900 by default, F11 toggles fullscreen)
static bool wantFullscreen = false;    // what the player asked for
static bool appliedFullscreen = false; // what is currently on screen

// puts the window in the wanted mode; does nothing if it is already in that mode
// (safe to call when a screen starts, e.g. game_init used to force the window size)
void window_mode_apply(void) {
	if (wantFullscreen) {
		if (!appliedFullscreen) {
			CP_System_FullscreenAdvanced(GAME_WIDTH, GAME_HEIGHT);
			appliedFullscreen = true;
		}
	}
	else if (appliedFullscreen || CP_System_GetWindowWidth() != GAME_WIDTH || CP_System_GetWindowHeight() != GAME_HEIGHT) {
		// leaving fullscreen, or the first time the window is set up: 1600 x 900, centred on the screen
		appliedFullscreen = false;
		CP_System_SetWindowSize(GAME_WIDTH, GAME_HEIGHT);
		CP_System_SetWindowPosition((CP_System_GetDisplayWidth() - GAME_WIDTH) / 2, (CP_System_GetDisplayHeight() - GAME_HEIGHT) / 2);
	}
}

// runs every frame on every screen (set with CP_Engine_SetPreUpdateFunction in main.c)
void window_mode_update(void) {
	if (CP_Input_KeyTriggered(KEY_F11)) {
		wantFullscreen = !wantFullscreen;
		window_mode_apply();
	}
}

// ---------------------------------------------------------------------------
// TEXT WIDTH
// advance width of each character ' ' (32) to '~' (126) in WickedScaryMovie, as a fraction of the text size
// (read from the font file; the font has no kerning, so adding these up gives the exact width)
static const float bodyFontAdvance[95] = {
	0.189f, 0.137f, 0.210f, 0.354f, 0.303f, 0.476f, 0.248f, 0.095f, 0.159f, 0.159f,
	0.318f, 0.248f, 0.110f, 0.275f, 0.112f, 0.288f, 0.319f, 0.165f, 0.312f, 0.313f,
	0.319f, 0.314f, 0.313f, 0.278f, 0.316f, 0.318f, 0.118f, 0.111f, 0.553f, 0.292f,
	0.561f, 0.294f, 0.768f, 0.313f, 0.318f, 0.314f, 0.321f, 0.271f, 0.271f, 0.325f,
	0.323f, 0.125f, 0.303f, 0.349f, 0.242f, 0.449f, 0.323f, 0.319f, 0.306f, 0.340f,
	0.320f, 0.317f, 0.246f, 0.321f, 0.280f, 0.454f, 0.348f, 0.321f, 0.313f, 0.445f,
	0.288f, 0.445f, 0.286f, 0.418f, 0.136f, 0.314f, 0.324f, 0.313f, 0.312f, 0.314f,
	0.252f, 0.380f, 0.323f, 0.120f, 0.142f, 0.316f, 0.125f, 0.497f, 0.330f, 0.317f,
	0.312f, 0.312f, 0.213f, 0.315f, 0.281f, 0.318f, 0.280f, 0.454f, 0.347f, 0.313f,
	0.301f, 0.159f, 0.833f, 0.159f, 0.000f,
};

// same for Darkening (titleFont), already scaled the way CProcessing sizes this font
static const float titleFontAdvance[95] = {
	0.161f, 0.106f, 0.139f, 0.257f, 0.222f, 0.331f, 0.225f, 0.077f, 0.156f, 0.154f,
	0.202f, 0.214f, 0.096f, 0.194f, 0.096f, 0.130f, 0.213f, 0.160f, 0.213f, 0.214f,
	0.244f, 0.221f, 0.221f, 0.213f, 0.228f, 0.222f, 0.096f, 0.096f, 0.170f, 0.254f,
	0.179f, 0.182f, 0.355f, 0.205f, 0.218f, 0.200f, 0.229f, 0.195f, 0.181f, 0.261f,
	0.213f, 0.097f, 0.202f, 0.214f, 0.210f, 0.281f, 0.222f, 0.204f, 0.197f, 0.222f,
	0.221f, 0.226f, 0.244f, 0.219f, 0.218f, 0.298f, 0.225f, 0.204f, 0.211f, 0.157f,
	0.132f, 0.164f, 0.242f, 0.251f, 0.113f, 0.205f, 0.218f, 0.200f, 0.229f, 0.195f,
	0.181f, 0.261f, 0.213f, 0.097f, 0.202f, 0.214f, 0.210f, 0.281f, 0.222f, 0.204f,
	0.197f, 0.222f, 0.221f, 0.226f, 0.244f, 0.219f, 0.218f, 0.298f, 0.225f, 0.204f,
	0.211f, 0.186f, 0.094f, 0.166f, 0.279f,
};

// advance width of one character as a fraction of the text size
static float char_advance(CP_Font font, char c) {
	if (c < 32 || c > 126) {
		return 0.0f;
	}
	return (font == titleFont) ? titleFontAdvance[c - 32] : bodyFontAdvance[c - 32];
}

// width in pixels of a line of text drawn in the given font (titleFont or bodyFont) at the given size
float font_text_width(CP_Font font, const char* text, float size) {
	float width = 0.0f;
	for (const char* c = text; *c != '\0'; c++) {
		width += char_advance(font, *c);
	}
	return width * size;
}

// width in pixels of a line of text drawn in bodyFont at the given size
float body_text_width(const char* text, float size) {
	return font_text_width(bodyFont, text, size);
}

// draws a line of text centred on centerX with extra space between the letters
// (CProcessing has no letter-spacing setting, so each character is drawn on its own).
// spacing is a fraction of the text size, e.g. 0.13f. Uses the current fill colour.
void draw_spaced_text(CP_Font font, const char* text, float centerX, float y, float size, float spacing) {
	int count = 0;
	for (const char* c = text; *c != '\0'; c++) {
		count++;
	}
	float gap = spacing * size;
	float totalWidth = font_text_width(font, text, size) + gap * (count > 0 ? count - 1 : 0);
	float x = centerX - totalWidth / 2;

	CP_Font_Set(font);
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	for (const char* c = text; *c != '\0'; c++) {
		char letter[2] = { *c, '\0' };
		CP_Font_DrawText(letter, x, y);
		x += char_advance(font, *c) * size + gap;
	}
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
}

// true if the mouse is over a line of bodyFont text (vertically centred on y, aligned horizontally by align)
bool text_hovered(const char* text, float x, float y, float size, CP_TEXT_ALIGN_HORIZONTAL align) {
	float width = body_text_width(text, size);
	float left = x;
	if (align == CP_TEXT_ALIGN_H_CENTER) left = x - width / 2;
	else if (align == CP_TEXT_ALIGN_H_RIGHT) left = x - width;

	return is_button_hovered(left, y - size / 2, width, size);
}

// ---------------------------------------------------------------------------
// TEXT BUTTONS (shared hover animation used by every screen)
#define BUTTON_HOVER_TIME       0.15f   // seconds to ease in/out of the hover look
#define BUTTON_HOVER_SCALE      0.15f   // text grows by 15% when hovered
#define BUTTON_CLICK_TIME       0.1f    // seconds of the click shrink
#define BUTTON_CLICK_SHRINK     0.12f   // text shrinks by up to 12% when clicked
#define BUTTON_SHADOW_OFFSET    3.0f    // shadow is drawn 3px down and right
#define BUTTON_SHADOW_ALPHA     153     // 60% black

static float buttonAlpha = 1.0f; // multiplies the alpha of every text button (see text_button_set_alpha)

static float random_range(float min, float max) {
	return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

// eases 0..1 smoothly (slow start, slow end)
static float smooth_step(float t) {
	return t * t * (3.0f - 2.0f * t);
}

static int lerp_channel(int from, int to, float t) {
	return (int)((float)from + (float)(to - from) * t);
}

void text_button_reset(TextButton* button) {
	button->hoverT = 0.0f;
	button->clickTimer = 0.0f;
	button->flickerWait = 0.0f;
	button->dipTimer = 0.0f;
	button->dipStrength = 1.0f;
}

// candle flicker: while hovered the text is steady, but every so often it dips in brightness for a split second,
// sometimes twice in a row like a sputtering flame
static void text_button_flicker(TextButton* button, bool hovered, float dt) {
	if (!hovered) {
		button->dipTimer = 0.0f;
		button->flickerWait = random_range(0.6f, 1.6f); // first dip comes a little after the hover starts
		return;
	}

	if (button->dipTimer > 0.0f) {
		button->dipTimer -= dt;
		return;
	}

	button->flickerWait -= dt;
	if (button->flickerWait <= 0.0f) {
		button->dipTimer = random_range(0.04f, 0.1f);      // short dip
		button->dipStrength = random_range(0.35f, 0.65f);  // how dark the dip gets
		if (rand() % 4 == 0) {
			button->flickerWait = random_range(0.06f, 0.14f); // quick second dip
		}
		else {
			button->flickerWait = random_range(0.9f, 2.4f);   // long steady stretch
		}
	}
}

// fades every text button drawn after this call (1 = fully visible). Set it back to 1 when done.
// It only changes how the buttons look; they can still be hovered and clicked.
void text_button_set_alpha(float alpha) {
	buttonAlpha = (alpha < 0.0f) ? 0.0f : ((alpha > 1.0f) ? 1.0f : alpha);
}

// slow, gentle version of the candle flicker for titles: long steady stretches with an occasional soft dip
void title_flicker_reset(TextButton* state) {
	text_button_reset(state);
	state->flickerWait = random_range(1.5f, 3.0f);
}

// returns the title's brightness this frame (1 = steady)
float title_flicker_update(TextButton* state) {
	float dt = CP_System_GetDt();
	if (state->dipTimer > 0.0f) {
		state->dipTimer -= dt;
		return state->dipStrength;
	}
	state->flickerWait -= dt;
	if (state->flickerWait <= 0.0f) {
		state->dipTimer = random_range(0.08f, 0.18f);    // short, soft dip
		state->dipStrength = random_range(0.6f, 0.8f);   // not as dark as the buttons' flicker
		state->flickerWait = random_range(2.5f, 5.0f);   // and much less often
		return state->dipStrength;
	}
	return 1.0f;
}

// how hovered the button looks right now, eased from 0 (normal) to 1 (fully hovered)
float text_button_hover_amount(const TextButton* button) {
	return smooth_step(button->hoverT);
}

// draws a clickable line of bodyFont text with the shared hover animation:
// eased grow + colour fade to ember on hover, candle flicker, drop shadow, and a quick shrink when clicked.
// forceHover lets a screen treat the button as hovered (e.g. when the mouse is over a picture that belongs to it).
// returns true once, when a click's shrink animation has finished (that is when the screen should act on it)
bool text_button_update(TextButton* button, const char* text, float x, float y, float size, CP_TEXT_ALIGN_HORIZONTAL align, bool forceHover) {
	float dt = CP_System_GetDt();
	bool hovered = forceHover || text_hovered(text, x, y, size, align); // hitbox uses the normal (not enlarged) size
	bool activated = false;

	// hover ease (0.15s each way)
	if (hovered) {
		button->hoverT += dt / BUTTON_HOVER_TIME;
		if (button->hoverT > 1.0f) button->hoverT = 1.0f;
	}
	else {
		button->hoverT -= dt / BUTTON_HOVER_TIME;
		if (button->hoverT < 0.0f) button->hoverT = 0.0f;
	}
	float ease = smooth_step(button->hoverT);

	// click shrink (0.1s), the button activates when it finishes
	if (button->clickTimer > 0.0f) {
		button->clickTimer -= dt;
		if (button->clickTimer <= 0.0f) {
			button->clickTimer = 0.0f;
			activated = true;
		}
	}
	else if (hovered && CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT)) {
		button->clickTimer = BUTTON_CLICK_TIME;
	}
	float clickShrink = 0.0f;
	if (button->clickTimer > 0.0f) {
		float progress = 1.0f - button->clickTimer / BUTTON_CLICK_TIME;
		clickShrink = BUTTON_CLICK_SHRINK * sinf(progress * 3.14159265f); // shrink and come back
	}

	text_button_flicker(button, hovered, dt);
	float brightness = (button->dipTimer > 0.0f) ? button->dipStrength : 1.0f;

	// colour: pale grey-white (#D9D4CC) -> dim ember orange (#E07A3A)
	int r = lerp_channel(217, 224, ease);
	int g = lerp_channel(212, 122, ease);
	int b = lerp_channel(204, 58, ease);
	CP_Color textColor = CP_Color_Create((int)(r * brightness), (int)(g * brightness), (int)(b * brightness), (int)(255 * buttonAlpha));

	float drawSize = size * (1.0f + BUTTON_HOVER_SCALE * ease) * (1.0f - clickShrink);

	CP_Font_Set(bodyFont);
	CP_Settings_TextAlignment(align, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_TextSize(drawSize);

	// soft dark shadow behind the text
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)(BUTTON_SHADOW_ALPHA * buttonAlpha)));
	CP_Font_DrawText(text, x + BUTTON_SHADOW_OFFSET, y + BUTTON_SHADOW_OFFSET);

	CP_Settings_Fill(textColor);
	CP_Font_DrawText(text, x, y);

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	return activated;
}

// "[Esc] Back" prompt in the bottom-left corner: a yellow key box (same style as the Controls screen keys)
// followed by the word "Back". The whole prompt is clickable; the word uses the shared hover animation
// and the box brightens slightly on hover. Returns true when it has been clicked or Esc is pressed.
// (only used on Guide, Controls, Mode Select and Credits, so Esc keeps pausing during gameplay)
bool back_button_update(TextButton* button) {
	float boxWidth = body_text_width("Esc", BACK_KEY_TEXT_SIZE) + 2.0f * BACK_KEY_BOX_PADDING;
	float boxTop = BACK_BUTTON_Y - BACK_KEY_BOX_HEIGHT / 2;
	float wordX = BACK_BUTTON_X + boxWidth + BACK_KEY_GAP;

	// key box (drawn first so the word sits next to it)
	float hover = text_button_hover_amount(button);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(lerp_channel(241, 255, hover), lerp_channel(194, 218, hover), lerp_channel(50, 100, hover), 255));
	CP_Graphics_DrawRectAdvanced(BACK_BUTTON_X, boxTop, boxWidth, BACK_KEY_BOX_HEIGHT, 0.0f, 12.0f);

	CP_Font_Set(bodyFont);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_TextSize(BACK_KEY_TEXT_SIZE);
	CP_Settings_Fill(CP_Color_Create(30, 30, 30, 255)); // dark key text
	CP_Font_DrawText("Esc", BACK_BUTTON_X + boxWidth / 2, BACK_BUTTON_Y);

	// hovering the box counts as hovering the prompt (the word checks its own hitbox)
	bool boxHovered = is_button_hovered(BACK_BUTTON_X, boxTop, boxWidth + BACK_KEY_GAP, BACK_KEY_BOX_HEIGHT);
	bool clicked = text_button_update(button, "Back", wordX, BACK_BUTTON_Y, BACK_BUTTON_SIZE, CP_TEXT_ALIGN_H_LEFT, boxHovered);
	return clicked || CP_Input_KeyTriggered(KEY_ESCAPE);
}
