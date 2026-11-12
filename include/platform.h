/**
 * @file        platform.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the platform struct, the platform list and the platform
 *              functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "utils.h"

extern CP_Image Platforms;

#define MAX_PLATFORMS 13

typedef struct {
	float x, y;
	float width, height;
	Hitbox hitbox;
} Platform;

extern Platform platforms[MAX_PLATFORMS];

void platform_init();
void platform_draw();

