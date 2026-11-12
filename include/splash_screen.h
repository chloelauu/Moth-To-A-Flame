/**
 * @file        splash_screen.h
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       Declares the splash screen's init, update and exit functions.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

// Function prototypes for the splash screen state
void splash_init(void);    // Initializes the splash screen
void splash_update(void);  // Updates the splash screen (fade-in, logo display, fade-out)
void splash_exit(void);    // Cleans up resources used in the splash screen
