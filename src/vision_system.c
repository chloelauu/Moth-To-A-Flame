/**
 * @file        vision_system.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Camera zoom and follow, and in Hard mode the shrinking light
 *              around the player that recharges near light sources and ends the
 *              game when it runs out.
 * @par         Polish (2026): Chloe Lau Rey En. Resets the light message on
 *              restart.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/vision_system.h"
#include "../include/light_source.h"
#include "../include/game.h"
#include "../include/mode_select.h"
#include <math.h>

VisionSystem visionSystem;

static float gameOverTimer = 0.0f;
float lightsourceMessageDuration = 0.0f;
bool lightsourceShowMessage = false;


void vision_system_init(void) {
	// reset the "light growing stronger" message so a restart starts exactly like the first game
	lightsourceShowMessage = false;
	lightsourceMessageDuration = 0.0f;

	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		visionSystem.isGameOver = false;
		visionSystem.zoomFactor = 4.0f;
		visionSystem.currentRadius = MAX_RADIUS;
		return; // No initialization needed for "Without Vision" mode
	}

	visionSystem.x = player.x;
	visionSystem.y = player.y;
	visionSystem.currentRadius = MAX_RADIUS;
	visionSystem.depleteRate = DEPLETION_RATE;
	visionSystem.rechargeRate = RECHARGE_RATE;
	visionSystem.zoomFactor = 1.5f;
	visionSystem.isGameOver = false;
}

void vision_system_update(void) {
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		return; // Skip vision radius updates
	}

	if (!isPaused) {
		float dt = CP_System_GetDt();

		visionSystem.x = player.x;
		visionSystem.y = player.y;

		if (is_near_light_source()) {
			//printf("Player is near a light source. Increasing vision size.\n");
			lightsourceShowMessage = true;
			lightsourceMessageDuration = 0.0f;
			visionSystem.currentRadius += visionSystem.rechargeRate * dt;
			if (visionSystem.currentRadius > MAX_RADIUS) {
				visionSystem.currentRadius = MAX_RADIUS;
			}
		}
		else {
			visionSystem.currentRadius -= visionSystem.depleteRate * dt;
			if (visionSystem.currentRadius <= MIN_RADIUS) {
				visionSystem.currentRadius = MIN_RADIUS;
				visionSystem.isGameOver = true;
				playerDeathCause = DEATH_BY_DARKNESS;
				currentState = STATE_GAME_OVER;  // Transition to the game-over state
			}

		}
	}
	else {
		visionSystem.x = player.x;
		visionSystem.y = player.y;
	}
}

void vision_system_begin(void) {
	float windowWidth = (float)CP_System_GetWindowWidth();
	float windowHeight = (float)CP_System_GetWindowHeight();

	// Calculate visible area
	float visibleWidth = windowWidth / visionSystem.zoomFactor;
	float visibleHeight = windowHeight / visionSystem.zoomFactor;

	CP_Settings_ResetMatrix();

	// Center the view
	CP_Settings_Translate(windowWidth / 2.0f, windowHeight / 2.0f);

	// Apply zoom
	CP_Settings_Scale(visionSystem.zoomFactor, visionSystem.zoomFactor);

	// Move to player position (with Y position properly handled)
	float targetY = player.y;

	CP_Settings_Translate(-player.x, -targetY);
}

void vision_system_end(void) {
	CP_Settings_ResetMatrix();
}

void vision_system_draw(void) {
	if (currentState == STATE_MAIN_MENU || selectedMode == GAME_MODE_WITHOUT_VISION) {
		return;
	}

	if (!visionSystem.isGameOver) {
		// Save current matrix state
		CP_Settings_Save();

		// Reset to screen space for overlay
		CP_Settings_ResetMatrix();

		float windowWidth = (float)CP_System_GetWindowWidth();
		float windowHeight = (float)CP_System_GetWindowHeight();

		// Set the side length for both width and height to create a square cutout
		float sideLength = (float)0.7 * visionSystem.currentRadius * visionSystem.zoomFactor;

		// Ensure the side length does not exceed the smallest window dimension to keep it square
		if (sideLength > windowWidth) sideLength = windowWidth;
		if (sideLength > windowHeight) sideLength = windowHeight;

		// Calculate the top-left corner to center the cutout on the screen
		float cutoutX = (windowWidth - sideLength) / 2.0f;
		float cutoutY = (windowHeight - sideLength) / 2.0f;

		// Draw dark rectangles around the cutout area to create the clear effect
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));  // Semi-transparent dark overlay
		CP_Settings_NoStroke();

		// Top rectangle (above the cutout)
		CP_Graphics_DrawRect(0, 0, windowWidth, cutoutY);

		// Bottom rectangle (below the cutout)
		CP_Graphics_DrawRect(0, cutoutY + sideLength, windowWidth, windowHeight - (cutoutY + sideLength));

		// Left rectangle (left of the cutout)
		CP_Graphics_DrawRect(0, cutoutY, cutoutX, sideLength);

		// Right rectangle (right of the cutout)
		CP_Graphics_DrawRect(cutoutX + sideLength, cutoutY, windowWidth - (cutoutX + sideLength), sideLength);

		// Restore previous matrix state
		CP_Settings_Restore();
	}
}

void vision_system_set_zoom(float zoom) {
	visionSystem.zoomFactor = zoom;
}

void vision_system_exit(void) {
}