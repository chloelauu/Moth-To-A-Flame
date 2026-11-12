/**
 * @file        hazards.h
 * @project     Moth To A Flame
 * @author      Tan Jia Qi Trish
 *              Chloe Lau Rey En
 * @brief       Declares the hazard types and structs, the hazard arrays and the
 *              hazard functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "../include/utils.h"
#include "../include/player.h"
#include <cprocessing.h>
#include <stdbool.h>

typedef enum {
	FIRE, // assigned value 0
	WASPNESTS // assigned value 1
} DamageHazardType;

typedef enum {
	COBWEBS, // assigned value 0
	HONEY // assigned value 1
} SlowingHazardType;

typedef struct {
	DamageHazardType type;
	float x, y;
	float damageValue;
	Hitbox hitbox;
	Hitbox safeZones;
} DamageHazard;

typedef struct {
	SlowingHazardType type;
	float x, y;
	float radius;
	Hitbox hitbox; 
} SlowingHazard;

// number of cobwebs
#define MAX_COBWEBS_HAZARDS 250

// Extern declarations for accessing these variables in other files
extern int activeHoneyHazards;
extern SlowingHazard honeyHazards[10];
extern SlowingHazard cobwebHazards[MAX_COBWEBS_HAZARDS];
extern DamageHazard fireHazards[3], beehiveHazards[2];

void hazard_init();
void hazard_update();
void hazard_draw();
void hazard_exit();