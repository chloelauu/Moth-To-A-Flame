/**
 * @file        bird.c
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 * @brief       The bird enemy: when the player is in range it flies back and
 *              forth between two platforms along a curved path, pausing briefly
 *              at each end.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/enemy.h"
#include "../include/player.h"

CP_Image birdSprite;

Enemy bird;
const float BIRD_SCALE = 60.0f; // size of bird sprite
const float PAUSE_DURATION = 0.1f; // pause duration in seconds
float speed = 8.0f;
float vertex_x, vertex_y;
float patrolStartX = 253.0f + 25.0f; // offset by 25.0f bc map platforms are drawn by CP_POSITION_CORNER
float patrolStartY = 370.0f + 50.0f;
// no patrolEndY since the parabola travels to same the same Y pos
float patrolEndX = 1190.0f + 25.0f; // offset by 25.0f bc map platforms are drawn by CP_POSITION_CORNER

// parabolic equation y = a(x − h)^2 + k
// (h,k) is the vertex of the parabola (the peak point)
float calculate_y_position(float x, float a, float h, float k) {
    return a * (x - h) * (x - h) + k;
}

void bird_init(float patrolStartX) {
    bird.x = patrolStartX;
    bird.y = patrolStartY;

    bird.speed = speed;
    bird.time = 0.0f; // Start time
    bird.isLanding = false;
    bird.direction = 1;          // facing right
    bird.isPaused = false;       // Initialize paused state to false
    bird.pauseTimer = 0.0f;      // Initialize pause timer

    // Initialize hitboxes
    bird.hitboxCollide.type = CIRCLE;
    bird.hitboxCollide.x = bird.x;
    bird.hitboxCollide.y = bird.y;
    bird.hitboxCollide.diameter = BIRD_SCALE - 10.0f;
    bird.hitboxCollide.radius = bird.hitboxCollide.diameter / 2;

    bird.hitboxAggro.type = CIRCLE;
    bird.hitboxAggro.x = bird.x;
    bird.hitboxAggro.y = bird.y;
    bird.hitboxAggro.diameter = 2000.0f;
    bird.hitboxAggro.radius = bird.hitboxAggro.diameter / 2;

    bird.state = ENEMY_IDLE; // set initial bird state as idle

    birdSprite = CP_Image_Load("Assets/Sprites/bird_flying/bird_flying1.png");
}

void bird_update(float patrolStartX, float patrolEndX) {
    vertex_x = (patrolStartX + patrolEndX) / 2; // midpoint of start and end point
    vertex_y = 500.0f; // set this to the peak height as desired

    // Calculate 'a' so the parabola lands at 'patrolStartY' at both endpoints
    float a = (patrolStartY - vertex_y) / ((patrolStartX - vertex_x) * (patrolStartX - vertex_x));

    // constantly check for aggro
    if (checkCollision(bird.hitboxAggro, player.hitbox)) {
        bird.state = ENEMY_PATROLLING; // switch bird state to patrolling
    }
    // bird only becomes idle if it reaches a platform -> doesn't stop midflight in the air
    else if (bird.x == patrolStartX || bird.x == patrolEndX) {
        bird.state = ENEMY_IDLE;
    }

    // bird stops moving when it is set to idle (speed set to 0)
    if (bird.state == ENEMY_IDLE) {
        bird.speed = 0.0f;
    }
    else if (bird.state == ENEMY_PATROLLING) {
        bird.speed = speed;
    }

    if (bird.isPaused) {
        // If bird is paused, increment the pause timer
        bird.pauseTimer += CP_System_GetDt();

        // Check if the pause duration has elapsed
        if (bird.pauseTimer >= PAUSE_DURATION) {
            bird.isPaused = false; // Resume movement
            bird.pauseTimer = 0.0f; // Reset the timer
        }
    }
    else {
        // Update position and check patrol bounds
        bird.x += bird.speed * bird.direction;

        // If bird reaches patrol endpoints, set to pause
        if (bird.x >= patrolEndX) {
            bird.x = patrolEndX;
            bird.direction = -1; // Reverse direction
            bird.isPaused = true; // Start pauses
        }
        else if (bird.x <= patrolStartX) {
            bird.x = patrolStartX;
            bird.direction = 1; // Reverse direction
            bird.isPaused = true; // Start pause
        }
    }

    // Calculate y position using parabolic motion
    bird.y = calculate_y_position(bird.x, a, vertex_x, vertex_y);

    // Update hitbox positions
    bird.hitboxCollide.x = bird.x;
    bird.hitboxCollide.y = bird.y;
    bird.hitboxAggro.x = bird.x;
    bird.hitboxAggro.y = bird.y;
}

void bird_draw() {
    // Draw hitbox for aggro testing
    //CP_Settings_Fill(CP_Color_Create(178, 3, 0, 100));
    //CP_Settings_Stroke(CP_Color_Create(178, 3, 0, 255));
    //CP_Graphics_DrawCircle(bird.hitboxAggro.x, bird.hitboxAggro.y, bird.hitboxAggro.diameter);

    // Draw hitbox for collision testing
    //CP_Settings_Fill(CP_Color_Create(237, 227, 176, 100));
    //CP_Settings_Stroke(CP_Color_Create(237, 227, 176, 255));
    //CP_Graphics_DrawCircle(bird.hitboxCollide.x, bird.hitboxCollide.y, bird.hitboxCollide.diameter);

    CP_Settings_ImageMode(CP_POSITION_CENTER);
    float DIRECTION_SCALE;

    // flip image to corresponding facing direction
    if (bird.direction == -1) {
        DIRECTION_SCALE = -BIRD_SCALE;
    }
    else {
        DIRECTION_SCALE = BIRD_SCALE;
    }
    CP_Image_Draw(birdSprite, bird.x, bird.y, DIRECTION_SCALE, BIRD_SCALE, 255);
}

void bird_exit() {
    CP_Image_Free(&birdSprite);
}