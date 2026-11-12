/**
 * @file        food.h
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 * @brief       Declares the food item struct and the food functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "utils.h"
#include <cprocessing.h>
#include <stdbool.h>

typedef struct {
	float x, y;
	float healthValue;
	bool isActive;
	Hitbox hitbox;
} FoodItem;

void food_init();
void food_update();
void food_draw();
void food_exit();