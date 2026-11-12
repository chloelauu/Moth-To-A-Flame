/**
 * @file        light_source.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the light source struct and functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include <stdbool.h>
#include "../include/utils.h"

typedef struct {
	float x, y;
	float radius;
	Hitbox hitbox;
} LightSource;

void lightsource_init();
void lightsource_update();
void lightsource_draw();
void lightsource_exit();

extern bool is_near_light_source(void);