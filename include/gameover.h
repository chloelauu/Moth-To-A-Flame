/**
 * @file        gameover.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the game over screen functions and its sound.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include <stdbool.h>
#include "cprocessing.h"

extern CP_Sound gameOverSound;
extern bool soundPlayed;

void game_over_init();
void game_over_update();
void game_over_exit();
