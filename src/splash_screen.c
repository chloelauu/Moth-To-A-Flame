/**
 * @file        splash_screen.c
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       The DigiPen splash screen that fades in and out before the main
 *              menu; it also loads the game's fonts and sets up the window.
 * @par         Polish (2026): Chloe Lau Rey En. Loads the shared fonts once and
 *              opens the game in a centred window.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/splash_screen.h"
#include "../include/mainmenu.h"
#include "../include/utils.h"

static CP_Image backgroundImage;
CP_Color myColor;

// Variables for managing fade and timing
static float alpha = 0.0f; // Transparency level
static int fade_in = 1;    // 1 for fade-in, 0 for fade-out
static float display_time = 1.0f; // Time to display the logo (in seconds)
static float timer = 0.0f;

void splash_init(void) {
    // start in a centred 1600 x 900 window (F11 toggles fullscreen, see utils.c)
    window_mode_apply();

    // load the shared title/body fonts once for the whole game (the splash screen itself draws no text)
    fonts_load();

    alpha = 0.0f;
    fade_in = 1;
    timer = 0.0f;

    // Optionally load a logo texture here
    backgroundImage = CP_Image_Load("Assets/Backgrounds/splashscreen.jpg");
    myColor = CP_Color_Create(255, 255, 255, 255);
}

void splash_update(void)
{
    // Background color (black screen)
    CP_Settings_Fill(myColor);
    CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
    CP_Settings_ImageMode(CP_POSITION_CENTER);

    // Fade-in logic
    if (fade_in)
    {
        alpha += 255.0f * CP_System_GetDt(); // Increase alpha over time
        if (alpha >= 255.0f)
        {
            alpha = 255.0f;
            fade_in = 0; // Switch to logo display
        }
    }
    else if (timer < display_time)
    {
        timer += CP_System_GetDt(); // Wait for the logo display time
    }
    else
    {
        alpha -= 255.0f * CP_System_GetDt(); // Decrease alpha for fade-out
        if (alpha <= 0.0f)
        {
            // Transition to the main menu once fade-out is complete
            CP_Engine_SetNextGameState(main_menu_init, main_menu_update, main_menu_exit);
        }
    }

    // Draw the logo image with the current alpha transparency
    CP_Image_DrawAdvanced(
        backgroundImage,
        (float)CP_System_GetWindowWidth() / 2, // Center X
        (float)CP_System_GetWindowHeight() / 2, // Center Y
        1600.0f,  // Width
        900.0f,  // Height
        (int)alpha, // Alpha transparency
        0.0f       // Rotation in degrees
    );
}

void splash_exit(void) {
    CP_Image_Free(&backgroundImage);
}