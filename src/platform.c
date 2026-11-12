/**
 * @file        platform.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       The tree platforms: their positions, hitboxes and drawing.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/platform.h"
#include "../include/game.h"

CP_Color platformColor;

Platform platforms[MAX_PLATFORMS] = {
	// first tree
	{203.0f, 643.0f, 50.0f, 10.0f},  // format: x, y, width, height 
	{287.0f, 620.0f, 50.0f, 10.0f},
	{253.0f, 450.0f, 50.0f, 10.0f},

	// second tree
	{590.0f, 633.0f, 50.0f, 10.0f},
	{558.0f, 460.0f, 50.0f, 10.0f},
	{682.0f, 600.0f, 50.0f, 10.0f},
	{640.0f, 405.0f, 50.0f, 10.0f},

	// third tree
	{892.0f, 658.0f, 50.0f, 10.0f},
	{941.0f, 495.0f, 50.0f, 10.0f},
	{971.0f, 623.0f, 50.0f, 10.0f},

	// fourth tree
	{1245.0f, 635.0f, 50.0f, 10.0f},
	{1190.0f, 450.0f, 50.0f, 10.0f},
	{1348.0f, 603.0f, 50.0f, 10.0f}
};

void platform_init() {
	platformColor = CP_Color_Create(160, 97, 36, 255);
	// creates hitboxes for all platforms
	for (int i = 0; i < MAX_PLATFORMS; i++) {
		platforms[i].hitbox.type = RECTANGLE;
		platforms[i].hitbox.y = platforms[i].y;
		platforms[i].hitbox.x = platforms[i].x;
		platforms[i].hitbox.width = platforms[i].width;
		platforms[i].hitbox.height = platforms[i].height;
	}
}

void platform_draw() {
	extern CP_Image Platforms;

	for (int i = 0; i < MAX_PLATFORMS; i++) {
		CP_Settings_Fill(platformColor);
		CP_Settings_ImageMode(CP_POSITION_CORNER);
		CP_Image_Draw(Platforms, platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height, 255);

#if 0
		// debug hitbox visualization
		CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
		CP_Settings_StrokeWeight(2.0f);
		CP_Graphics_DrawRect(platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height);
#endif
	}
}