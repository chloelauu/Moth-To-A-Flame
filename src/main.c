/**
 * @file        main.c
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 * @brief       Program entry point: seeds the random numbers, turns on the F11
 *              fullscreen toggle and starts the game at the splash screen.
 * @par         Polish (2026): Chloe Lau Rey En. F11 fullscreen toggle hooked up
 *              for every screen.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game.h"
#include "../include/mainmenu.h"
#include "../include/splash_screen.h"
#include "../include/utils.h"

#include <time.h>
#include <stdlib.h> // Include for srand and rand

int main(void)
{
	srand((unsigned int)time(NULL)); // Set the random seed once at the start

	// F11 toggles fullscreen on every screen (see window_mode_update in utils.c)
	CP_Engine_SetPreUpdateFunction(window_mode_update);

	// Set the initial game state to the splash screen
	CP_Engine_SetNextGameState(splash_init, splash_update, splash_exit);
	CP_Engine_Run(0);	
	return 0;
}