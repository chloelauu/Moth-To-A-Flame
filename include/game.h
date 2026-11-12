/**
 * @file        game.h
 * @project     Moth To A Flame
 * @author      Chloe Lau Rey En
 *              Tan Shi Qi Charity
 * @brief       Declares the gameplay state functions, the death causes, the
 *              ground hitboxes and shared game state.
 * @par         Polish (2026): Chloe Lau Rey En. Removed the old
 *              restartedFromCredits flag.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "utils.h"
#include "../include/game_state.h"  // Include game states
#include "../include/vision_system.h"
#include "../include/light_source.h"

void game_init(void);
void game_update(void);
void game_exit(void);
void game_logic_update(void);

typedef enum {
    DEATH_BY_DARKNESS,  // Player died due to vision radius depletion
    DEATH_BY_HAZARDS    // Player died due to health depletion
} DeathCause;

// Declare the death cause globally
extern DeathCause playerDeathCause;

// for ground checking
extern Hitbox groundHitboxLower;  // Lower (main) ground level
extern Hitbox groundHitboxUpper;  // Upper ground level
extern float groundTop;        // Make groundTop accessible
extern float upperGroundTop;   // Make upperGroundTop accessible

// game state
extern bool gameRestarted;
extern GameState currentState;

extern CP_Sound gameMusic;
