/**
 * @file        game_state.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Declares the list of game states (main menu, mode select,
 *              gameplay, game over, credits and others).
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

typedef void(__cdecl* GameStateFunc)(void);

typedef enum {
	STATE_MAIN_MENU,
	STATE_MODE_SELECT,
	STATE_TUTORIAL,
	STATE_CONTROLS,
	STATE_GAME,
	STATE_GAME_OVER,
	STATE_GAME_COMPLETED
} GameState;

extern GameState currentState;
extern int reachedFromGameplay;
