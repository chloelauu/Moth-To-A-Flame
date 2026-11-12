/**
 * @file        light_source.c
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 * @brief       Mushrooms, lamps and fire safe zones that recharge the player's
 *              light in Hard mode, with a sound when the player enters one.
 * @par         Polish (2026): Chloe Lau Rey En. Frees the light sound, and the
 *              light zone state is reset on restart.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/light_source.h"
#include "../include/player.h"
#include "../include/hazards.h"
#include "../include/mode_select.h"

CP_Image lampSprite, mushroomGreenSprite, mushroomRedSprite, mushroomBlueSprite;
CP_Sound lightSource;

LightSource lamp1, lamp2, mushroom1, mushroom2;
static bool isInLightZone = false; // Tracks if the player is in a light zone
float LAMP_SCALE = 70.0f;
float MUSHROOM_SCALE = 80.0f;

void lightsource_init() {
	isInLightZone = false; // so the light sound plays when the player starts inside a light zone, like the first game

	mushroom1.x = 100.0f;
	mushroom1.y = 708.0f;

	mushroom2.x = 1500.0f;
	mushroom2.y = 708.0f;

	lamp1.x = 665.0f;
	lamp1.y = 445.0f;

	lamp2.x = 966.0f;
	lamp2.y = 535.0f;

	// initialize hitboxes
	mushroom1.hitbox.type = CIRCLE;
	mushroom1.hitbox.x = mushroom1.x;
	mushroom1.hitbox.y = mushroom1.y;
	mushroom1.hitbox.diameter = (float)(MUSHROOM_SCALE * 2.5);
	mushroom1.hitbox.radius = mushroom1.hitbox.diameter / 2;

	mushroom2.hitbox.type = CIRCLE;
	mushroom2.hitbox.x = mushroom2.x;
	mushroom2.hitbox.y = mushroom2.y;
	mushroom2.hitbox.diameter = (float)(MUSHROOM_SCALE * 2.5);
	mushroom2.hitbox.radius = mushroom2.hitbox.diameter / 2;

	lamp1.hitbox.type = CIRCLE;
	lamp1.hitbox.x = lamp1.x;
	lamp1.hitbox.y = lamp1.y;
	lamp1.hitbox.diameter = (float)(LAMP_SCALE * 2.5);
	lamp1.hitbox.radius = lamp1.hitbox.diameter / 2;

	lamp2.hitbox.type = CIRCLE;
	lamp2.hitbox.x = lamp2.x;
	lamp2.hitbox.y = lamp2.y + 10.0f;
	lamp2.hitbox.diameter = LAMP_SCALE;
	lamp2.hitbox.radius = lamp2.hitbox.diameter / 2;

	lampSprite = CP_Image_Load("Assets/Sprites/lightsources/hanginglamp.png");
	mushroomGreenSprite = CP_Image_Load("Assets/Sprites/lightsources/mushroom_green.png");
	mushroomRedSprite = CP_Image_Load("Assets/Sprites/lightsources/mushroom_red.png");
	mushroomBlueSprite = CP_Image_Load("Assets/Sprites/lightsources/mushroom_blue.png");

	lightSource = CP_Sound_Load("Assets/Audio/safe_zones/holy_light_1.wav");
}

void lightsource_update() {
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		return; // Skip updates for "Without Vision" mode
	}
}

void lightsource_draw() {
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		// Draw only the mushrooms, skip radius circles
		CP_Image_Draw(mushroomGreenSprite, mushroom1.x, mushroom1.y, -MUSHROOM_SCALE, MUSHROOM_SCALE, 255);
		CP_Image_Draw(mushroomBlueSprite, mushroom2.x, mushroom2.y, MUSHROOM_SCALE, MUSHROOM_SCALE, 255);
		CP_Image_Draw(mushroomRedSprite, 120.0f, 716.0f, (float)(MUSHROOM_SCALE / 1.5), (float)(MUSHROOM_SCALE / 1.5), 255); // Left mushroom
		CP_Image_Draw(mushroomGreenSprite, 1480.0f, 716.0f, (float)(MUSHROOM_SCALE / 1.5), (float)(MUSHROOM_SCALE / 1.5), 255); // Right mushroom
		CP_Image_Draw(lampSprite, lamp1.x, lamp1.y, LAMP_SCALE, LAMP_SCALE, 255);
		CP_Image_Draw(lampSprite, lamp2.x, lamp2.y, LAMP_SCALE, LAMP_SCALE, 255);
		return; // Skip radius circles and lamps
	}

	// Draw mushrooms
	CP_Image_Draw(mushroomGreenSprite, mushroom1.x, mushroom1.y, -MUSHROOM_SCALE, MUSHROOM_SCALE, 255);
	CP_Image_Draw(mushroomBlueSprite, mushroom2.x, mushroom2.y, MUSHROOM_SCALE, MUSHROOM_SCALE, 255);
	CP_Image_Draw(mushroomRedSprite, 120.0f, 716.0f, (float)(MUSHROOM_SCALE / 1.5), (float)(MUSHROOM_SCALE / 1.5), 255);
	CP_Image_Draw(mushroomGreenSprite, 1480.0f, 716.0f, (float)(MUSHROOM_SCALE / 1.5), (float)(MUSHROOM_SCALE / 1.5), 255);

	// Draw lamps
	CP_Image_Draw(lampSprite, lamp1.x, lamp1.y, LAMP_SCALE, LAMP_SCALE, 255);
	CP_Image_Draw(lampSprite, lamp2.x, lamp2.y, LAMP_SCALE, LAMP_SCALE, 255);

	// Draw light radius circles
	CP_Settings_Fill(CP_Color_Create(237, 227, 176, 100));
	CP_Settings_Stroke(CP_Color_Create(237, 227, 176, 255));
	CP_Graphics_DrawCircle(mushroom1.hitbox.x, mushroom1.hitbox.y, mushroom1.hitbox.diameter);
	CP_Graphics_DrawCircle(mushroom2.hitbox.x, mushroom2.hitbox.y, mushroom1.hitbox.diameter);
	CP_Graphics_DrawCircle(lamp1.hitbox.x, lamp1.hitbox.y, lamp1.hitbox.diameter);
	CP_Graphics_DrawCircle(lamp2.hitbox.x, lamp2.hitbox.y, lamp2.hitbox.diameter);

}

void lightsource_exit() {
	CP_Image_Free(&lampSprite);
	CP_Image_Free(&mushroomGreenSprite);
	CP_Image_Free(&mushroomRedSprite);
	CP_Image_Free(&mushroomBlueSprite);
	CP_Sound_Free(&lightSource);
}

bool is_near_light_source() {
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		return false; // No light source effects in "Without Vision" mode
	}
	bool currentlyInLightZone = false;

	// Check collision with mushroom1 and play sound if entering
	if (checkCollision(player.hitbox, mushroom1.hitbox)) {
		currentlyInLightZone = true;
		if (!isInLightZone) {
			CP_Sound_Play(lightSource);
		}
	}
	// Check collision with mushroom2 and play sound if entering
	else if (checkCollision(player.hitbox, mushroom2.hitbox)) {
		currentlyInLightZone = true;
		if (!isInLightZone) {
			CP_Sound_Play(lightSource);
		}
	}
	// Check collision with lamp1 and play sound if entering
	else if (checkCollision(player.hitbox, lamp1.hitbox)) {
		currentlyInLightZone = true;
		if (!isInLightZone) {
			CP_Sound_Play(lightSource);
		}
	}
	// Check collision with lamp2 and play sound if entering
	else if (checkCollision(player.hitbox, lamp2.hitbox)) {
		currentlyInLightZone = true;
		if (!isInLightZone) {
			CP_Sound_Play(lightSource);
		}
	}
	// Check collision with fire hazards' safe zones and play sound if entering
	else {
		for (int i = 0; i < 3; i++) {
			if (checkCollision(player.hitbox, fireHazards[i].safeZones)) {
				currentlyInLightZone = true;
				if (!isInLightZone) {
					CP_Sound_Play(lightSource);
				}
				break;
			}
		}
	}

	isInLightZone = currentlyInLightZone; // Update the state
	return currentlyInLightZone;
}
