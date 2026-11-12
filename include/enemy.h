/**
 * @file        enemy.h
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 *              Choi Meng Yew
 * @brief       Declares the shared enemy struct and states, and the bird and
 *              frog functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "utils.h"
#include "player.h"

typedef enum { ENEMY_IDLE, ENEMY_PATROLLING } EnemyState;       // SHARED ENEMY ENUM
typedef enum { JUMP_FORWARD, JUMP_BACKWARD } JumpDirection;     // FROG ENUM

// GENERAL ENEMY STRUCT
typedef struct {
    float x, y;
    float damage;
    Hitbox hitboxCollide; // one hitbox for checking if enemy collides with player
    Hitbox hitboxAggro; // one hitbox for checking enemy distance from player
    EnemyState state;

    float direction;
    float speed;         // Speed of the bird's movement
    float time;          // Time variable to control the flight path
    float pauseTimer;
    bool isLanding;      // Flag to check if the bird is landing
    bool isPaused;
} Enemy;

extern Enemy bird, frog;
extern float patrolStartX, patrolEndX; // for bird

void frog_init(float startX, float startY);
void frog_draw();
void frog_update(float deltaTime);
void frog_exit();

float calculate_y_position(float x, float a, float h, float k);
void bird_init(float patrolStartX);
void bird_update(float patrolStartX, float patrolEndX);
void bird_draw();
void bird_exit();