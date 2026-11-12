/**
 * @file        vision_system.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the vision system struct, the light radius constants
 *              and its functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "cprocessing.h"
#include "../include/player.h"
#include "../include/light_source.h"

typedef struct {
    float x, y;
    float currentRadius;
    float rechargeRate;
    float depleteRate;
    float zoomFactor;
    bool isGameOver;
} VisionSystem;

// Constants
#define MAX_RADIUS 150.0f
#define MIN_RADIUS 0.0f
#define DEPLETION_RATE 15.0f
#define RECHARGE_RATE 20.0f

// External declarations
extern VisionSystem visionSystem;
extern bool isPaused;
extern float lightsourceMessageDuration;
extern bool lightsourceShowMessage;

// Function declarations
void vision_system_init(void);
void vision_system_update(void);
void vision_system_begin(void);
void vision_system_end(void);
void vision_system_draw(void);
void vision_system_set_zoom(float zoom);
void vision_system_exit(void);