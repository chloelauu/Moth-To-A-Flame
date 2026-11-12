/**
 * @file        player.h
 * @project     Moth To A Flame
 * @author      Tan Shi Qi Charity
 *              Tan Jia Qi Trish
 * @brief       Declares the player struct, the maximum health and the player
 *              functions.
 * @par         Polish (2026): Chloe Lau Rey En. Added PLAYER_MAX_HEALTH.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include <stdbool.h>
#include "utils.h"

typedef struct {
	float x, y;	// position coordinates
	float speed; // speed
	float health;
	Hitbox hitbox;
} Player;

#define PLAYER_MAX_HEALTH 5.0f // starting and maximum health (5 lives)

extern Player player, mate;

void player_init();
void player_update();
void player_flight();
void player_crawl();
void stamina();
void drawStaminaBar();
void dashCooldown();
void health();
void mate_player_collision();
void player_draw();
void guide();
void mate_draw();
void player_exit();

// functions for checking if player is touching the cobweb or not
bool is_in_slowing_hazards(float playerX, float playerY, float playerRadius);