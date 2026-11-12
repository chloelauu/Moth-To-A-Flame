/**
 * @file        food.c
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 * @brief       Apples placed at random spots on the map each game; touching one
 *              restores a heart (up to the maximum) and shows a short message.
 * @par         Polish (2026): Chloe Lau Rey En. Resets the apple message on
 *              restart and uses PLAYER_MAX_HEALTH.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/food.h"
#include "../include/player.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX_SPAWN_LOCATIONS 14 // possible spawn locations
#define MAX_FOOD_ITEMS 7 // number of sprites that will actually spawn in one game

CP_Image foodSprite;
CP_Sound foodSound;

const float FOOD_SCALE = 70.0f;

// array to hold possible spawn locations (on top of platforms and on the ground)
const float spawn_x[MAX_SPAWN_LOCATIONS] = { 228.0f, 312.0f, 278.0f, 615.0f, 583.0f, 707.0f, 665.0f, 966.0f, 996.0f, 1270.0f, 1373.0f, 287.0f, 590.0f, 1190.0f }; // offset by 25.0f bc map platforms are drawn by CP_POSITION_CORNER
const float spawn_y[MAX_SPAWN_LOCATIONS] = { 618.0f, 595.0f, 425.0f, 608.0f, 435.0f, 575.0f, 380.0f, 470.0f, 598.0f, 610.0f, 578.0f, 715.0f, 715.0f, 715.0f };

// array to hold food items to be spawned
FoodItem food_items[MAX_FOOD_ITEMS]; 

float messageDuration = 0.0f;
bool showMessage = false;

void food_init() {
    // reset the "magical apple" message so a restart starts exactly like the first game
    showMessage = false;
    messageDuration = 0.0f;

    int usedLocations[MAX_SPAWN_LOCATIONS] = { 0 }; // tracks used locations so the sprites don't spawn twice at the same spot

    for (int i = 0; i < MAX_FOOD_ITEMS; i++) {
        int index;
        do {
            index = rand() % MAX_SPAWN_LOCATIONS;
        } while (usedLocations[index] == 1); // generate new location if location already used

        // mark as used by assigning the value in the index as 1
        usedLocations[index] = 1;

        food_items[i].x = spawn_x[index];
        food_items[i].y = spawn_y[index];
        food_items[i].healthValue = 1.0f;
        food_items[i].isActive = true;

        // Initialize hitboxes
        food_items[i].hitbox.type = CIRCLE; // Ensure your Hitbox structure can support this
        food_items[i].hitbox.x = food_items[i].x;
        food_items[i].hitbox.y = food_items[i].y;
        food_items[i].hitbox.diameter = FOOD_SCALE - 10.0f;
        food_items[i].hitbox.radius = food_items[i].hitbox.diameter / 2.5f;

        // Remove hitbox drawing from here
    }

    // Load assets
    foodSprite = CP_Image_Load("Assets/Sprites/food_sprites/apple.png");
    foodSound = CP_Sound_Load("Assets/Audio/food_collection/food_collect.wav");
}

void food_update() {
    for (int i = 0; i < MAX_FOOD_ITEMS; i++) {
        if (food_items[i].isActive) {
            if (checkCollision(player.hitbox, food_items[i].hitbox)) {
                CP_Sound_Play(foodSound);
                // Activate the message and reset its duration
                showMessage = true;
                messageDuration = 0.0f;

                food_items[i].isActive = false;

                // Remove the hitbox
                food_items[i].hitbox.x = 0;
                food_items[i].hitbox.y = 0;
                food_items[i].hitbox.diameter = 0;

                // Add healthValue to the player (only if the player is not at full health)
                if (player.health < PLAYER_MAX_HEALTH) {
                    player.health += food_items[i].healthValue;
                }
            }
        }
    }

    // Handle message display
    if (showMessage) {
        messageDuration += CP_System_GetDt();
        if (messageDuration < 1.0f) {
            CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
            CP_Settings_TextSize(FONT_SIZE_BODY);
            CP_Font_DrawText("This magical apple seems to heal your wounds instantly...", (float)CP_System_GetWindowWidth() / 2, 70.0f);
        }
        else {
            showMessage = false;
        }
    }
}

// Draw function to render food and hitboxes
void food_draw() {
    CP_Settings_ImageMode(CP_POSITION_CENTER);
    for (int i = 0; i < MAX_FOOD_ITEMS; i++) {
        if (food_items[i].isActive) {
            // Draw hitbox for collision testing
            //CP_Settings_Fill(CP_Color_Create(61, 255, 29, 100));
            //CP_Settings_Stroke(CP_Color_Create(61, 255, 29, 255));
            //CP_Graphics_DrawCircle(food_items[i].hitbox.x, food_items[i].hitbox.y, food_items[i].hitbox.diameter / 2); // Adjusted to draw with radius

            CP_Image_Draw(foodSprite, food_items[i].x, food_items[i].y, FOOD_SCALE, FOOD_SCALE, 255);
        }
    }
}

void food_exit() {
    CP_Sound_Free(&foodSound);
    CP_Image_Free(&foodSprite); // Free the image
}