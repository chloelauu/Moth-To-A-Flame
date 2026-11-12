/**
 * @file        mode_select.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the game modes (Easy and Hard) and the selected mode.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

typedef enum {
	GAME_MODE_NONE,
	GAME_MODE_WITH_VISION,
	GAME_MODE_WITHOUT_VISION
} GameMode;

extern GameMode selectedMode;

void mode_select_init(void);
void mode_select_update(void);
void mode_select_exit(void);