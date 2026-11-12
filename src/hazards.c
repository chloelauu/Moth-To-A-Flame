/**
 * @file        hazards.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En: sticky hazards (honey, cobwebs)
 *              Tan Jia Qi Trish: damage hazards (fire, beehives)
 * @brief       Places and draws the hazards: fire and beehives that cost
 *              health, and honey and cobwebs that slow the player down.
 * @par         Polish (2026): Chloe Lau Rey En. Debug hitbox circles hidden.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/player.h"
#include "../include/hazards.h"
#include "../include/mode_select.h"

CP_Image fireSprite, beehiveSprite, cobwebSprite, cobwebSideSprite, honeySprite; 

const float FIRE_SCALE = 100.0f, BEEHIVE_SCALE = 70.0F;
int activeHoneyHazards = 5; // Default for "With Vision" moded

// positioning of hazards
DamageHazard fireHazards[3] = { 0 }, beehiveHazards[2] = { 0 }; // spawn in 3 fire hazards and 2 beehive hazards
SlowingHazard honeyHazards[10] = { 0 };
SlowingHazard cobwebHazards[MAX_COBWEBS_HAZARDS] = {
	// left edge
	{COBWEBS, 20.0f, 0.0f, 20.0f},
	{COBWEBS, 20.0f, 30.0f, 20.0f},
	{COBWEBS, 20.0f, 60.0f, 20.0f},
	{COBWEBS, 20.0f, 90.0f, 20.0f},
	{COBWEBS, 20.0f, 120.0f, 20.0f},
	{COBWEBS, 20.0f, 150.0f, 20.0f},
	{COBWEBS, 20.0f, 180.0f, 20.0f},
	{COBWEBS, 20.0f, 210.0f, 20.0f},
	{COBWEBS, 20.0f, 240.0f, 20.0f},
	{COBWEBS, 20.0f, 270.0f, 20.0f},
	{COBWEBS, 20.0f, 300.0f, 20.0f},
	{COBWEBS, 20.0f, 330.0f, 20.0f},
	{COBWEBS, 20.0f, 360.0f, 20.0f},
	{COBWEBS, 20.0f, 390.0f, 20.0f},
	{COBWEBS, 20.0f, 420.0f, 20.0f},
	{COBWEBS, 20.0f, 450.0f, 20.0f},
	{COBWEBS, 20.0f, 480.0f, 20.0f},
	{COBWEBS, 20.0f, 510.0f, 20.0f},
	{COBWEBS, 20.0f, 540.0f, 20.0f},
	{COBWEBS, 20.0f, 570.0f, 20.0f},
	{COBWEBS, 20.0f, 600.0f, 20.0f},
	{COBWEBS, 20.0f, 630.0f, 20.0f},
	{COBWEBS, 20.0f, 660.0f, 20.0f},

	// top edge
{COBWEBS, 55.0f, 0.0f, 20.0f},
{COBWEBS, 90.0f, 0.0f, 20.0f},
{COBWEBS, 125.0f, 0.0f, 20.0f},
{COBWEBS, 160.0f, 0.0f, 20.0f},
{COBWEBS, 195.0f, 0.0f, 20.0f},
{COBWEBS, 230.0f, 0.0f, 20.0f},
{COBWEBS, 265.0f, 0.0f, 20.0f},
{COBWEBS, 245.0f, 0.0f, 20.0f},
{COBWEBS, 275.0f, 0.0f, 20.0f},
{COBWEBS, 305.0f, 0.0f, 20.0f},
{COBWEBS, 335.0f, 0.0f, 20.0f},
{COBWEBS, 365.0f, 0.0f, 20.0f},
{COBWEBS, 395.0f, 0.0f, 20.0f},
{COBWEBS, 425.0f, 0.0f, 20.0f},
{COBWEBS, 455.0f, 0.0f, 20.0f},
{COBWEBS, 485.0f, 0.0f, 20.0f},
{COBWEBS, 515.0f, 0.0f, 20.0f},
{COBWEBS, 545.0f, 0.0f, 20.0f},
{COBWEBS, 575.0f, 0.0f, 20.0f},
{COBWEBS, 605.0f, 0.0f, 20.0f},
{COBWEBS, 635.0f, 0.0f, 20.0f},
{COBWEBS, 665.0f, 0.0f, 20.0f},
{COBWEBS, 695.0f, 0.0f, 20.0f},
{COBWEBS, 725.0f, 0.0f, 20.0f},
{COBWEBS, 755.0f, 0.0f, 20.0f},
{COBWEBS, 785.0f, 0.0f, 20.0f},
{COBWEBS, 815.0f, 0.0f, 20.0f},
{COBWEBS, 845.0f, 0.0f, 20.0f},
{COBWEBS, 875.0f, 0.0f, 20.0f},
{COBWEBS, 905.0f, 0.0f, 20.0f},
{COBWEBS, 935.0f, 0.0f, 20.0f},
{COBWEBS, 965.0f, 0.0f, 20.0f},
{COBWEBS, 995.0f, 0.0f, 20.0f},
{COBWEBS, 1025.0f, 0.0f, 20.0f},
{COBWEBS, 1055.0f, 0.0f, 20.0f},
{COBWEBS, 1085.0f, 0.0f, 20.0f},
{COBWEBS, 1115.0f, 0.0f, 20.0f},
{COBWEBS, 1145.0f, 0.0f, 20.0f},
{COBWEBS, 1175.0f, 0.0f, 20.0f},
{COBWEBS, 1205.0f, 0.0f, 20.0f},
{COBWEBS, 1235.0f, 0.0f, 20.0f},
{COBWEBS, 1265.0f, 0.0f, 20.0f},
{COBWEBS, 1295.0f, 0.0f, 20.0f},
{COBWEBS, 1325.0f, 0.0f, 20.0f},
{COBWEBS, 1355.0f, 0.0f, 20.0f},
{COBWEBS, 1385.0f, 0.0f, 20.0f},
{COBWEBS, 1415.0f, 0.0f, 20.0f},
{COBWEBS, 1445.0f, 0.0f, 20.0f},
{COBWEBS, 1475.0f, 0.0f, 20.0f},
{COBWEBS, 1505.0f, 0.0f, 20.0f},
{COBWEBS, 1535.0f, 0.0f, 20.0f},

// left corner cobwebs
{COBWEBS, 60.0f, 45.0f, 20.0f},
{COBWEBS, 50.0f, 60.0f, 15.0f},
{COBWEBS, 50.0f, 30.0f, 15.0f},
{COBWEBS, 80.0f, 33.0f, 15.0f},

// right corner cobwebs
{COBWEBS, 1505.0f, 30.0f, 20.0f},
{COBWEBS, 1480.0f, 25.0f, 15.0f},
{COBWEBS, 1500.0f, 25.0f, 15.0f},
{COBWEBS, 1520.0f, 60.0f, 15.0f},

// right edge
{COBWEBS, 1555.0f, 660.0f, 20.0f},
{COBWEBS, 1555.0f, 630.0f, 20.0f},
{COBWEBS, 1555.0f, 600.0f, 20.0f},
{COBWEBS, 1555.0f, 570.0f, 20.0f},
{COBWEBS, 1555.0f, 540.0f, 20.0f},
{COBWEBS, 1555.0f, 510.0f, 20.0f},
{COBWEBS, 1555.0f, 480.0f, 20.0f},
{COBWEBS, 1555.0f, 450.0f, 20.0f},
{COBWEBS, 1555.0f, 420.0f, 20.0f},
{COBWEBS, 1555.0f, 390.0f, 20.0f},
{COBWEBS, 1555.0f, 360.0f, 20.0f},
{COBWEBS, 1555.0f, 330.0f, 20.0f},
{COBWEBS, 1555.0f, 300.0f, 20.0f},
{COBWEBS, 1555.0f, 270.0f, 20.0f},
{COBWEBS, 1555.0f, 240.0f, 20.0f},
{COBWEBS, 1555.0f, 210.0f, 20.0f},
{COBWEBS, 1555.0f, 180.0f, 20.0f},
{COBWEBS, 1555.0f, 150.0f, 20.0f},
{COBWEBS, 1555.0f, 120.0f, 20.0f},
{COBWEBS, 1555.0f, 90.0f, 20.0f},
{COBWEBS, 1555.0f, 60.0f, 20.0f},
{COBWEBS, 1555.0f, 30.0f, 20.0f},

// random cobwebs
{COBWEBS, 1000.0f, 200.0f, 27.0f},
{COBWEBS, 1302.0f, 250.0f, 22.0f},
{COBWEBS, 532.0f, 210.0f, 23.0f},
{COBWEBS, 741.0f, 230.0f, 24.0f},
{COBWEBS, 320.0f, 370.0f, 12.0f},
{COBWEBS, 125.0f, 225.0f, 30.0f},
{COBWEBS, 380.0f, 110.0f, 25.0f},
};

void hazard_init() {
	// Adjust the number of honey hazards and positions based on the selected mode
	if (selectedMode == GAME_MODE_WITHOUT_VISION) {
		activeHoneyHazards = 6; // More honey hazards for "Without Vision" mode

		// Set positions for "Without Vision" mode
		honeyHazards[0].x = 315.0f; honeyHazards[0].y = 623.0f;
		honeyHazards[1].x = 665.0f; honeyHazards[1].y = 410.0f;
		honeyHazards[2].x = 995.0f; honeyHazards[2].y = 627.0f;
		honeyHazards[3].x = 1375.0f; honeyHazards[3].y = 606.0f;
		honeyHazards[4].x = 640.0f; honeyHazards[4].y = 737.0f;
		honeyHazards[5].x = 1240.0f; honeyHazards[5].y = 737.0f;
	}
	else {
		activeHoneyHazards = 5; // Fewer honey hazards for "With Vision" mode

		// Set positions for "With Vision" mode
		honeyHazards[0].x = 230.0f; honeyHazards[0].y = 647.0f;
		honeyHazards[1].x = 665.0f; honeyHazards[1].y = 410.0f;
		honeyHazards[2].x = 1375.0f; honeyHazards[2].y = 606.0f;
		honeyHazards[3].x = 740.0f; honeyHazards[3].y = 737.0f;
		honeyHazards[4].x = 1240.0f; honeyHazards[4].y = 737.0f;
	}

	// Initialize honey hazards
	for (int i = 0; i < activeHoneyHazards; i++) {
		honeyHazards[i].type = HONEY;
		honeyHazards[i].radius = 20.0f + (i % 2) * 5.0f; // Variable radius for variety
		honeyHazards[i].hitbox.type = CIRCLE;
		honeyHazards[i].hitbox.x = honeyHazards[i].x;
		honeyHazards[i].hitbox.y = honeyHazards[i].y;
		honeyHazards[i].hitbox.radius = honeyHazards[i].radius;
	}

	// damage hazards (fire, beehives)
	fireHazards[0].x = 450.0f;
	fireHazards[0].y = 708.0f;
	fireHazards[1].x = 917.0f;
	fireHazards[1].y = 628.0f;
	fireHazards[2].x = 1215.0f;
	fireHazards[2].y = 420.0f;

	beehiveHazards[0].x = 583.0f;
	beehiveHazards[0].y = 495.0f;
	beehiveHazards[1].x = 996.0f;
	beehiveHazards[1].y = 658.0f;

	// initialize hitboxes (damage hazards)
	for (int i = 0; i < 3; i++) {
		// for damage
		fireHazards[i].hitbox.type = CIRCLE;
		fireHazards[i].hitbox.x = fireHazards[i].x;
		fireHazards[i].hitbox.y = fireHazards[i].y + 10.0f;
		fireHazards[i].hitbox.radius = FIRE_SCALE / 10;

		// for recharging of light
		fireHazards[i].safeZones.type = CIRCLE;
		fireHazards[i].safeZones.x = fireHazards[i].x;
		fireHazards[i].safeZones.y = fireHazards[i].y + 10.0f;
		fireHazards[i].safeZones.radius = FIRE_SCALE / 2;
	}

	for (int i = 0; i < 2; i++) {
		beehiveHazards[i].hitbox.type = CIRCLE;
		beehiveHazards[i].hitbox.x = beehiveHazards[i].x;
		beehiveHazards[i].hitbox.y = beehiveHazards[i].y;
		beehiveHazards[i].hitbox.radius = BEEHIVE_SCALE / 10;
	}

	// initialize hitboxes (slowing hazards)
	for (int i = 0; i < MAX_COBWEBS_HAZARDS; i++) {
		cobwebHazards[i].hitbox.type = CIRCLE;
		cobwebHazards[i].hitbox.y = cobwebHazards[i].y;
		cobwebHazards[i].hitbox.x = cobwebHazards[i].x;
		cobwebHazards[i].hitbox.radius = cobwebHazards[i].radius;
	}

	// hazards sprite loader
	fireSprite = CP_Image_Load("Assets/Sprites/lightsources/flame.png");
	beehiveSprite = CP_Image_Load("Assets/Sprites/hazards/beehive.png");
	honeySprite = CP_Image_Load("Assets/Sprites/hazards/honey.png");
	cobwebSprite = CP_Image_Load("Assets/Sprites/hazards/cobweb.png");
	cobwebSideSprite = CP_Image_Load("Assets/Sprites/hazards/cobweb_side.png");
}

void hazard_update() {

}

void hazard_draw() {
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 200));
	CP_Settings_NoStroke();

	// slowing hazards
	for (int i = 0; i < MAX_COBWEBS_HAZARDS; i++) {
		CP_Image_Draw(cobwebSprite, cobwebHazards[i].x, cobwebHazards[i].y, cobwebHazards[i].radius * 2, cobwebHazards[i].radius * 2, 255);
	}

	for (int i = 0; i < activeHoneyHazards; i++) {  // Use activeHoneyHazards here
		CP_Image_Draw(honeySprite, honeyHazards[i].x, honeyHazards[i].y,
			honeyHazards[i].radius * 2, honeyHazards[i].radius * 2, 255);
	}

	// damage hazards
	for (int i = 0; i < 3; i++) {
#if 0
		// Draw hitbox for collision testing (fire)
		CP_Settings_Fill(CP_Color_Create(61, 255, 29, 100));
		CP_Settings_Stroke(CP_Color_Create(61, 255, 29, 255));
		CP_Graphics_DrawCircle(fireHazards[i].hitbox.x, fireHazards[i].hitbox.y, fireHazards[i].hitbox.radius);
#endif

		CP_Image_Draw(fireSprite, fireHazards[i].x, fireHazards[i].y, FIRE_SCALE, FIRE_SCALE, 255);

		if (selectedMode == GAME_MODE_WITH_VISION) {
			CP_Settings_Fill(CP_Color_Create(237, 227, 176, 100));
			CP_Settings_Stroke(CP_Color_Create(237, 227, 176, 255));
			CP_Graphics_DrawCircle(fireHazards[i].safeZones.x, fireHazards[i].safeZones.y, fireHazards[i].safeZones.radius * 2);
		}
	}

	for (int i = 0; i < 2; i++) {
#if 0
		// Draw hitbox for collision testing (beehive)
		CP_Settings_Fill(CP_Color_Create(61, 255, 29, 100));
		CP_Settings_Stroke(CP_Color_Create(61, 255, 29, 255));
		CP_Graphics_DrawCircle(beehiveHazards[i].hitbox.x, beehiveHazards[i].hitbox.y, beehiveHazards[i].hitbox.radius);
#endif

		CP_Image_Draw(beehiveSprite, beehiveHazards[i].x, beehiveHazards[i].y, BEEHIVE_SCALE, BEEHIVE_SCALE, 255);
	}
}

void hazard_exit() {
	CP_Image_Free(&fireSprite);
	CP_Image_Free(&beehiveSprite);
	CP_Image_Free(&cobwebSprite);
	CP_Image_Free(&cobwebSideSprite);
	CP_Image_Free(&honeySprite);
}