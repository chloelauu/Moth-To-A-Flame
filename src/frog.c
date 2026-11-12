/**
 * @file        frog.c
 * @project     Moth To A Flame
 * @author      Choi Meng Yew
 * @brief       The frog enemy: hops between points along the ground and shoots
 *              its tongue at the player when they come close, taking a heart on
 *              a hit.
 * @par         Polish (2026): Chloe Lau Rey En. Full reset of the frog on
 *              restart, the hit message now shows, and the lick sound plays on
 *              attacks with a cooldown so it never overlaps.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/enemy.h"
#include "../include/player.h"
#include "../include/vision_system.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CP_Image frogSprite;
CP_Sound frogLick;

Enemy frog;
const float FROG_SCALE = 60.0f; // size of frog sprite

// Frog-specific states
static EnemyState frogState = ENEMY_IDLE;
static JumpDirection jumpDirection = JUMP_FORWARD;

static int currentJumpPoint = 0;
static float jumpProgress = 0.0f;
static float jumpCooldown = 0.0f;

static const float jumpPoints[6][2] = {
    {650.0f, 715.0f},
    {775.0f, 715.0f},
    {920.0f, 715.0f},
    {1030.0f, 715.0f},
    {1150.0f, 715.0f},
    {1260.0f, 715.0f},
};

float ATTACK_RADIUS = 250.0f;       // Set the desired attack radius
static float JUMP_HEIGHT = 50.0f;   // Maximum height of the jump
static float JUMP_DURATION = 1.0f;  // Total time for the jump
static float JUMP_PEAK = 0.5f;      // Time at which the frog reaches its peak (0.5 corresponds to halfway through the jump)

// Tongue-specific properties
static const float TONGUE_MAX_LENGTH = 250.0f;  // Maximum reach of the tongue
static float tongueProgress = 0.0f;             // Tracks the extension and retraction of the tongue
static bool isTongueExtending = false;          // Tracks if the tongue is currently extending
static const float TONGUE_SPEED = 500.0f;       // Speed of the tongue's movement
static float targetX, targetY;                  // Target position for tongue extension

// Lick sound: plays when a tongue attack starts, but never on top of itself
// (CProcessing can't tell when a sound has finished, so wait the length of frog_lick.wav)
static const float LICK_SOUND_LENGTH = 3.3f;    // seconds (length of frog_lick.wav)
static const float LICK_SOUND_VOLUME = 0.7f;    // a little quieter than the other sound effects
static float lickSoundCooldown = 0.0f;

// Tongue cooldown properties
static float tongueCooldown = 0.0f;                 // Time remaining until the next tongue attack
static const float TONGUE_COOLDOWN_DURATION = 1.0f; // ADJUST FOR FASTER TONGUE ATTACKS

// Helper function
bool is_player_in_aggro_radius() {
    float distanceX = player.x - frog.x;
    float distanceY = player.y - frog.y;
    float distance = (float)sqrt(distanceX * distanceX + distanceY * distanceY);

    return distance <= ATTACK_RADIUS;
}

void start_tongue_attack() {
    isTongueExtending = true;
    tongueProgress = 0.0f; // Start extending from zero

    // Lock target position to player's last known position at time of attack
    targetX = player.x;
    targetY = player.y;
}

bool is_tongue_hitting_player(float tongueEndX, float tongueEndY) {
    // Get the vector from the frog to the tongue's endpoint
    float dx = tongueEndX - frog.x;
    float dy = tongueEndY - frog.y;

    // Get the vector from the frog to the player
    float px = player.x - frog.x;
    float py = player.y - frog.y;

    // Project player position onto the line of the tongue
    float projection = ((px * dx) + (py * dy)) / (dx * dx + dy * dy);

    // Clamp the projection to stay within the length of the tongue
    projection = (float)(fmax(0.0f, fmin(1.0f, projection)));

    // Find the closest point on the tongue line segment to the player
    float closestX = frog.x + projection * dx;
    float closestY = frog.y + projection * dy;

    // Calculate the distance from the closest point to the player
    float distanceX = closestX - player.x;
    float distanceY = closestY - player.y;
    float distance = (float)sqrt(distanceX * distanceX + distanceY * distanceY);

    // Check if the player is within a threshold distance (e.g., player radius or collision buffer)
    return distance <= 15.0f;
}

float frogTongueMessageDuration = 0.0f;
bool frogTongueShowMessage = false;

void frog_tongue_attack(float deltaTime) {
    if (isTongueExtending) {
        // Update tongue progress
        tongueProgress += deltaTime * TONGUE_SPEED;

        // Calculate angle from frog to target player position
        float angle = atan2f(targetY - frog.y, targetX - frog.x);

        // Calculate the current length of the tongue, clamping to TONGUE_MAX_LENGTH
        float tongueLength = (tongueProgress > TONGUE_MAX_LENGTH) ? TONGUE_MAX_LENGTH : tongueProgress;

        // Call `vision_system_begin()` here to apply the same transformations as the vision system
        vision_system_begin();

        // Set the stroke (outline) color and thickness
        CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255)); // Black outline color
        CP_Settings_StrokeWeight(2.0f);                    // Outline thickness
        
        // Calculate Y start position adjustment based on player location
        float tongueStartY = frog.y;
        if (targetX < frog.x) { // If player is on the left
            tongueStartY += 7.5f; // Adjust upwards slightly; tweak this as needed
        }

        // Draw the tongue using the transformed frog position and scaled tongue length
        CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
        CP_Graphics_DrawRectAdvanced(
            frog.x,                                // X start position of the tongue at the frog's position
            tongueStartY,                          // Y start position of the tongue at the frog's position
            tongueLength,                          // Width (length of the tongue)
            5.5f,                                  // Height (thickness of the tongue)
            (float)(angle * (180.0f / M_PI)),               // Convert angle to degrees for rotation
            5.5f                                   // Centered on the frog's position for correct alignment
        );

        // Call `vision_system_end()` to reset the transformation for other game elements
        vision_system_end();

        // Check for collision between tongue and player in world space
        float tongueEndX = frog.x + cosf(angle) * tongueLength;
        float tongueEndY = frog.y + sinf(angle) * tongueLength;
        if (is_tongue_hitting_player(tongueEndX, tongueEndY)) {
            // Handle the collision here (e.g., reduce player health or trigger an effect)
            player.health -= 1.0f;
            frogTongueShowMessage = true;
            frogTongueMessageDuration = 0.0f;
            isTongueExtending = false;  // Optionally retract tongue on hit
        }

        // Retract the tongue if it reaches max length
        if (tongueProgress >= TONGUE_MAX_LENGTH) {
            isTongueExtending = false;
        }
    }

    // Handle message display (outside "if (isTongueExtending)" so it stays up after the hit)
    if (frogTongueShowMessage) {
        frogTongueMessageDuration += CP_System_GetDt();
        if (frogTongueMessageDuration < 1.0f) {
            CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
            CP_Settings_TextSize(FONT_SIZE_BODY);
            CP_Font_DrawText("The frog lands a slimy attack on you!", (float)CP_System_GetWindowWidth() / 2, 220.0f);
        }
        else {
            frogTongueShowMessage = false;
        }
    }
}


void frog_init(float startX, float startY) {
    frogState = ENEMY_IDLE;
    frogLick = CP_Sound_Load("Assets/Audio/frog_tongue/frog_lick.wav");
    jumpCooldown = 1.0f;

    // reset the jump and tongue so a restart starts exactly like the first game
    jumpDirection = JUMP_FORWARD;
    frog.direction = 0;
    currentJumpPoint = 0;
    jumpProgress = 0.0f;
    tongueProgress = 0.0f;
    lickSoundCooldown = 0.0f;
    isTongueExtending = false;
    tongueCooldown = 0.0f;
    frogTongueShowMessage = false;
    frogTongueMessageDuration = 0.0f;
    frog.x = startX;
    frog.y = startY;

    // Initialize hitboxes
    frog.hitboxCollide.type = CIRCLE;
    frog.hitboxCollide.x = frog.x;
    frog.hitboxCollide.y = frog.y;
    frog.hitboxCollide.diameter = FROG_SCALE - 10.0f;
    frog.hitboxCollide.radius = frog.hitboxCollide.diameter / 2;

    frogSprite = CP_Image_Load("Assets/Sprites/frog_idle/frog_idle1.png");
}

void frog_update(float deltaTime) {
    bool inAggroRange = is_player_in_aggro_radius(frog, player);

    // Decrease the cooldown timer
    if (tongueCooldown > 0.0f) {
        tongueCooldown -= deltaTime;
    }
    if (lickSoundCooldown > 0.0f) {
        lickSoundCooldown -= deltaTime;
    }
    // Call the tongue attack function if the player is in range
    if (inAggroRange && !isTongueExtending && tongueCooldown <= 0.0f) {
        start_tongue_attack();
        tongueCooldown = TONGUE_COOLDOWN_DURATION;  // Reset cooldown

        // lick sound, only if the previous one has finished
        if (lickSoundCooldown <= 0.0f) {
            CP_Sound_PlayAdvanced(frogLick, LICK_SOUND_VOLUME, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
            lickSoundCooldown = LICK_SOUND_LENGTH;
        }
    }
    frog_tongue_attack(deltaTime);
    jumpCooldown -= deltaTime; // Decrease cooldown timer

    if (frogState == ENEMY_IDLE && jumpCooldown <= 0.0f) {
        // Start the jump if we have more points to jump to
        if ((jumpDirection == JUMP_FORWARD && currentJumpPoint < sizeof(jumpPoints) / sizeof(jumpPoints[0]) - 1) ||
            (jumpDirection == JUMP_BACKWARD && currentJumpPoint > 0)) {
            frogState = ENEMY_PATROLLING;
            jumpProgress = 0.0f; // Reset progress
            // Reset the cooldown timer with a random value (adjust as needed)
            jumpCooldown = (rand() % 1000) / 1000.0f + 0.5f; // Random time between 0.5 and 1.5 seconds
        }
    }

    if (frogState == ENEMY_PATROLLING) {
        // Increment the progress
        jumpProgress += deltaTime / JUMP_DURATION; // Change this value to control jump speed

        if (jumpProgress >= 1.0f) {
            // Jump is complete
            jumpProgress = 0.0f; // Reset progress for next jump
            frogState = ENEMY_IDLE; // Return to idle state
            jumpCooldown = (rand() % 1000) / 1000.0f + 0.5f; // Random cooldown time between 0.5 and 1.5 seconds

            // Update the current jump point based on direction
            if (jumpDirection == JUMP_FORWARD) {
                currentJumpPoint++; // Move to the next jump point
            }
            else {
                currentJumpPoint--; // Move to the previous jump point
            }

            // Check if we need to switch direction
            if (currentJumpPoint >= sizeof(jumpPoints) / sizeof(jumpPoints[0]) - 1) {
                jumpDirection = JUMP_BACKWARD; // Switch to backward direction
                frog.direction = -1;
                currentJumpPoint = sizeof(jumpPoints) / sizeof(jumpPoints[0]) - 1; // Clamp to the last point
            }
            else if (currentJumpPoint <= 0) {
                jumpDirection = JUMP_FORWARD; // Switch to forward direction
                frog.direction = 1;
                currentJumpPoint = 0; // Clamp to the first point
            }
        }

        // Calculate the frog's position based on jump progress
        float startX = jumpPoints[currentJumpPoint][0];
        float startY = jumpPoints[currentJumpPoint][1];
        float endX, endY;

        // Ensure we only access valid jump points
        if (jumpDirection == JUMP_FORWARD && currentJumpPoint < sizeof(jumpPoints) / sizeof(jumpPoints[0]) - 1) {
            endX = jumpPoints[currentJumpPoint + 1][0];
            endY = jumpPoints[currentJumpPoint + 1][1];
        }
        else if (jumpDirection == JUMP_BACKWARD && currentJumpPoint > 0) {
            endX = jumpPoints[currentJumpPoint - 1][0];
            endY = jumpPoints[currentJumpPoint - 1][1];
        }
        else {
            endX = startX; // Stay in place if there's no valid point
            endY = startY;
        }

        // Simple linear interpolation for X movement
        frog.x = startX + (endX - startX) * jumpProgress;

        // Calculate the corresponding Y position using a parabolic formula
        float t = jumpProgress; // Progress from 0 to 1
        float a = 4 * JUMP_HEIGHT; // Positive to create an upward-facing parabola

        // Calculate the vertex
        float h = JUMP_PEAK; // Midway point of the jump
        float k = startY; // Starting Y level

        // Parabola equation
        frog.y = a * (t - h) * (t - h) + k - JUMP_HEIGHT; // Adjust to subtract the height for landing

        // Adjust the final Y to ensure it lands on the correct ground level
        if (jumpProgress >= 1.0f) {
            frog.y = endY; // Reset Y position to ground level after landing
        }
    }

    // Update hitbox positions
    frog.hitboxCollide.x = frog.x;
    frog.hitboxCollide.y = frog.y;
    frog.hitboxAggro.x = frog.x;
    frog.hitboxAggro.y = frog.y;
}

void frog_draw() {
     //Draw hitbox for collision testing
    //CP_Settings_Fill(CP_Color_Create(0, 0, 255, 100));
    //CP_Settings_Stroke(CP_Color_Create(0, 0, 255, 255));
    //CP_Graphics_DrawCircle(frog.hitboxCollide.x, frog.hitboxCollide.y, frog.hitboxCollide.diameter);

    // Attack radius visualizer
#if 0
    CP_Color attackColor = CP_Color_Create(255, 0, 0, 50);
    CP_Settings_Fill(attackColor);
    CP_Settings_StrokeWeight(1.0f);
    CP_Graphics_DrawEllipse(frog->x, frog->y, ATTACK_RADIUS * 2, ATTACK_RADIUS * 2); // Draw attack radius outline
#endif 

    CP_Settings_ImageMode(CP_POSITION_CENTER);
    float DIRECTION_SCALE;

    // flip image to corresponding facing direction
    if (frog.direction == -1) {
        DIRECTION_SCALE = -FROG_SCALE;
    }
    else {
        DIRECTION_SCALE = FROG_SCALE;
    }
    CP_Image_Draw(frogSprite, frog.x, frog.y, DIRECTION_SCALE, FROG_SCALE, 255);
}

void frog_exit() {
    CP_Sound_Free(&frogLick);
    CP_Image_Free(&frogSprite);
}