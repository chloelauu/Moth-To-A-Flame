/**
 * @file        player.c
 * @project     Moth To A Flame
 * @author      Tan Shi Qi Charity: drawing the player, movement, stamina,
 *              player-mate and sticky hazard collisions
 *              Tan Jia Qi Trish: enemy and damage hazard collisions, health,
 *              fall damage, guide arrows and text cues
 *              Chloe Lau Rey En: sticky hazard collisions
 * @brief       The player moth: flying, crawling and dashing, stamina, health
 *              and fall damage, collisions with enemies, hazards and the mate,
 *              and the hearts, stamina bar, guide arrow and hazard messages.
 * @par         Polish (2026): Chloe Lau Rey En. Full reset of the player on
 *              restart, frees every image and sound it loads, win triggers only
 *              once, honey and cobweb messages fixed, PLAYER_MAX_HEALTH added.
 *
 * @copyright   Copyright (C) 2024 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "cprocessing.h"
#include "../include/player.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/enemy.h"
#include "../include/platform.h"
#include "../include/hazards.h"
#include "../include/game_state.h"
#include "../include/game_complete.h"
#include "../include/gameover.h"

CP_Image playerSpriteFrames[2], staminaBar, heartIcon, arrowIcon, mateSprite;
CP_Color staminaBarColor;

// for drawing player and mate
Player player, mate;
const float PLAYER_RADIUS = 12.0f;  // changes the COLLISION HITBOX of the sprite (not the size visually)
const float SPRITE_SCALE = 20.0f;   // This indicates the size of the sprite visually

// for player animation
int currentFrame = 0; // To track the current sprite frame
bool facingRight = true; // To track which direction the player is facing

// for stamina system
bool isOutOfStamina = false;
bool onCooldown = false;
float dashCooldownTimer = 0.0f; // track the remaining cooldown time
const float dashCooldownDuration = 3.0f; // 3 sec for cooldown time
float staminaBarX, staminaBarY, staminaBarWidth, staminaBarHeight, staminaTip1X, staminaTip1Y, staminaTip2X, staminaTip2Y, staminaTip3X, staminaTip3Y;

// for player enemy collisions
float collisionCooldown = 1.5f; // cooldown period for collisions (to prevent the player from taking continuous damage)
float birdCollisionTimer = 0.0f, frogCollisionTimer = 0.0f, fireCollisionTimer = 0.0f, beehiveCollisionTimer = 0.0f;

// for stamina system
static float currentStamina = 100.0f; // Only set initial value once, not every frame
static const float maxStamina = 100.0f;
static const float staminaDepletionRate = 25.0f;
static float verticalVelocity = 0.0f;

// for health() function, health system and fall damage
bool isFallingHealth = false, isLandingHealth = false, fallStartedHealth = false;
float initialYHealth = 0.0f, finalYHealth = 0.0f;

// for printing visual cue text messages
float fireMessageDuration = 0.0f, beehiveMessageDuration = 0.0f, honeyMessageDuration = 0.0f, cobwebMessageDuration = 0.0f;
bool fireShowMessage = false, beehiveShowMessage = false, honeyShowMessage = false, cobwebShowMessage = false;

static float gameOverTimer = 0.0f;

// audio management
CP_Sound fallDamage, playerCollision, gameComplete, gameOverHealth, gameOverSound; // frogLick is loaded in frog.c

void player_init() {
	player.x = 70.0f;
	player.y = 718.0f;
	player.speed = 1500.0f;
	player.health = PLAYER_MAX_HEALTH; // starting player health (5 lives)

	mate.x = 1440.0f;
	mate.y = upperGroundTop - PLAYER_RADIUS - 415.0f;

	currentStamina = maxStamina;

	// reset the rest of the player state so a restart starts exactly like the first game
	verticalVelocity = 0.0f;
	isOutOfStamina = false;
	onCooldown = false;
	dashCooldownTimer = 0.0f;
	birdCollisionTimer = 0.0f;
	frogCollisionTimer = 0.0f;
	fireCollisionTimer = 0.0f;
	beehiveCollisionTimer = 0.0f;
	isFallingHealth = false;
	isLandingHealth = false;
	fallStartedHealth = false;
	initialYHealth = 0.0f;
	finalYHealth = 0.0f;
	fireShowMessage = false;
	beehiveShowMessage = false;
	honeyShowMessage = false;
	cobwebShowMessage = false;
	fireMessageDuration = 0.0f;
	beehiveMessageDuration = 0.0f;
	honeyMessageDuration = 0.0f;
	cobwebMessageDuration = 0.0f;
	currentFrame = 0;
	facingRight = true;

	// Initialize player hitbox
	player.hitbox.type = CIRCLE;
	player.hitbox.radius = PLAYER_RADIUS;
	player.hitbox.x = player.x;
	player.hitbox.y = player.y;

	// Initalize mate hitbox
	mate.hitbox.type = CIRCLE;
	mate.hitbox.radius = PLAYER_RADIUS;
	mate.hitbox.x = mate.x;
	mate.hitbox.y = mate.y;

	playerSpriteFrames[0] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying1.png");
	playerSpriteFrames[1] = CP_Image_Load("Assets/Sprites/moth_flying/moth_flying2.png");
	mateSprite = CP_Image_Load("Assets/Sprites/mate_flying/mate_flying1.png");
	staminaBar = CP_Image_Load("Assets/Sprites/ui_icons/stamina_bar.png");
	heartIcon = CP_Image_Load("Assets/Sprites/ui_icons/heart_icon.png");
	arrowIcon = CP_Image_Load("Assets/Sprites/ui_icons/arrow_icon.png");

	fallDamage = CP_Sound_Load("Assets/Audio/Fall/fall_damage.wav");
	playerCollision = CP_Sound_Load("Assets/Audio/player_collisions/player_collision.wav");
	gameComplete = CP_Sound_Load("Assets/Audio/game_complete/win_music.wav");

	soundPlayed = false;  // Reset flag on restart
	gameOverSound = CP_Sound_Load("Assets/Audio/game_over/game_over.wav");
}

void player_update() {
	if (player.health <= 0) {
		CP_Sound_StopAll();
		playerDeathCause = DEATH_BY_HAZARDS;
		currentState = STATE_GAME_OVER;  // Transition directly to game over
	}

	float deltaTime = CP_System_GetDt();
	float previousY = player.y;  // Store previous position

	stamina();
	dashCooldown();
	// Movement controls
	player_flight();
	player_crawl();
	health();
	guide();
	mate_player_collision();

	// window boundary checks
	float windowWidth = (float)CP_System_GetWindowWidth();
	if (player.x - PLAYER_RADIUS < 0) {
		player.x = PLAYER_RADIUS;
	}
	if (player.x + PLAYER_RADIUS > windowWidth) {
		player.x = windowWidth - PLAYER_RADIUS;
	}

	if (player.y - PLAYER_RADIUS < 0) {
		player.y = PLAYER_RADIUS;
	}

	// Update hitbox position to match player position
	player.hitbox.x = player.x;
	player.hitbox.y = player.y;

	// Check collision with platforms first
	bool onPlatform = false;

	for (int i = 0; i < MAX_PLATFORMS; i++) {
		if (checkCollision(player.hitbox, platforms[i].hitbox)) {
			// Calculate the vertical overlap
			float playerBottom = player.y + PLAYER_RADIUS;
			float platformTop = platforms[i].y;
			float playerTop = player.y - PLAYER_RADIUS;
			float platformBottom = platforms[i].y + platforms[i].height;

			// Check if player is above the platform (coming from top)
			if (previousY + PLAYER_RADIUS <= platforms[i].y) {
				player.y = platforms[i].y - PLAYER_RADIUS;
				onPlatform = true;
			}
			// Check if player is below the platform (hitting from bottom)
			else if (previousY - PLAYER_RADIUS >= platforms[i].y + platforms[i].height) {
				player.y = platforms[i].y + platforms[i].height + PLAYER_RADIUS;
			}
			// Side collision
			else {
				// If coming from left
				if (player.x < platforms[i].x) {
					player.x = platforms[i].x - PLAYER_RADIUS;
				}
				// If coming from right
				else {
					player.x = platforms[i].x + platforms[i].width + PLAYER_RADIUS;
				}
			}

			// Update hitbox position after collision resolution
			player.hitbox.x = player.x;
			player.hitbox.y = player.y;
		}
	}

	// Only check upper ground collision if not on a platform
	if (!onPlatform && checkCollision(player.hitbox, groundHitboxUpper)) {
		player.y = upperGroundTop - PLAYER_RADIUS;
		player.hitbox.y = player.y;
	}

	// ENEMY COLLISIONS ----------------------------------------------------------------------------------------------------------------------------

		// BIRD ------------------------------------------------------------------------------------------------------------------------------------
		// cooldown to prevent continuous collision
	if (birdCollisionTimer > 0.0f) {
		birdCollisionTimer -= CP_System_GetDt();
	}

	// player & bird enemy collision checking
	if (birdCollisionTimer <= 0 && checkCollision(player.hitbox, bird.hitboxCollide)) {
		player.health -= 1.0f;
		CP_Sound_Play(playerCollision);
		//printf("Bird hit the player!\n");
		//printf("player health is now %.1f\n", player.health);

		// reset the timer to cooldown time
		birdCollisionTimer = collisionCooldown;
	}

	// FROG ------------------------------------------------------------------------------------------------------------------------------------
	// cooldown to prevent continuous collision
	if (frogCollisionTimer > 0.0f) {
		frogCollisionTimer -= CP_System_GetDt();
	}

	// player & frog enemy collision checking
	if (frogCollisionTimer <= 0 && checkCollision(player.hitbox, frog.hitboxCollide)) {
		CP_Sound_Play(playerCollision);
		player.health -= 1.0f; // player loses a heart of health when it collides into an enemy

		// reset the timer to cooldown time
		frogCollisionTimer = collisionCooldown;
	}

	// HAZARD COLLISIONS ----------------------------------------------------------------------------------------------------------------------------

		// FIRE -------------------------------------------------------------------------------------------------------------------------------------
		// cooldown to prevent continuous collision
	if (fireCollisionTimer > 0.0f) {
		fireCollisionTimer -= CP_System_GetDt();
	}

	for (int i = 0; i < 3; i++) {
		if (fireCollisionTimer <= 0 && checkCollision(player.hitbox, fireHazards[i].hitbox)) {
			player.health -= 1.0f;
			fireShowMessage = true;
			fireMessageDuration = 0.0f;

			// reset the timer to cooldown time
			fireCollisionTimer = collisionCooldown;
		}
	}

	// Handle message display
	if (fireShowMessage) {
		fireMessageDuration += CP_System_GetDt();
		if (fireMessageDuration < 1.0f) {
			CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
			CP_Settings_TextSize(FONT_SIZE_BODY);
			CP_Font_DrawText("Watch out for the flames!", (float)CP_System_GetWindowWidth() / 2, 170.0f);
		}
		else {
			fireShowMessage = false;
		}
	}

	// BEEHIVE ----------------------------------------------------------------------------------------------------------------------------------
	// cooldown to prevent continuous collision
	if (beehiveCollisionTimer > 0.0f) {
		beehiveCollisionTimer -= CP_System_GetDt();
	}
	for (int i = 0; i < 2; i++) {
		if (beehiveCollisionTimer <= 0 && checkCollision(player.hitbox, beehiveHazards[i].hitbox)) {
			player.health -= 1.0f;
			beehiveShowMessage = true;
			beehiveMessageDuration = 0.0f;

			// reset the timer to cooldown time
			beehiveCollisionTimer = collisionCooldown;
			// knock-back effect? on hit particle effect?
		}
	}

	// Handle message display
	if (beehiveShowMessage) {
		beehiveMessageDuration += CP_System_GetDt();
		if (beehiveMessageDuration < 1.0f) {
			CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
			CP_Settings_TextSize(FONT_SIZE_BODY);
			CP_Font_DrawText("You've angered the hive...", (float)CP_System_GetWindowWidth() / 2, 170.0f);
		}
		else {
			beehiveShowMessage = false;
		}
	}

	// LIGHT SOURCE -----------------------------------------------------------------------------------------------------------------------------
	if (lightsourceShowMessage) {
		lightsourceMessageDuration += CP_System_GetDt();
		if (lightsourceMessageDuration < 0.1f) {
			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
			CP_Settings_TextSize(FONT_SIZE_BODY);
			CP_Font_DrawText("You feel your light growing stronger...", (float)CP_System_GetWindowWidth() / 2, 120.0f);
		}
		else {
			lightsourceShowMessage = false;
		}
	}

	// Add debug visualization to see both collision boundaries
	/*CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawCircle(player.x, player.y, PLAYER_RADIUS * 2);
	CP_Graphics_DrawCircle(mate.x, mate.y, PLAYER_RADIUS * 2);*/
	//CP_Graphics_DrawRect(groundHitboxUpper.x, groundHitboxUpper.y, groundHitboxUpper.width, groundHitboxUpper.height);

	// Sprite animation logic
	static bool wasMovingLeft = false;
	static bool wasMovingRight = false;

	if (CP_Input_KeyReleased('A') && wasMovingLeft) {
		currentFrame = (currentFrame + 1) % 2;
		wasMovingLeft = false;
	}
	if (CP_Input_KeyDown('A')) {
		wasMovingLeft = true;
	}
	if (CP_Input_KeyReleased('D') && wasMovingRight) {
		currentFrame = (currentFrame + 1) % 2;
		wasMovingRight = false;
	}
	if (CP_Input_KeyDown('D')) {
		wasMovingRight = true;
	}
}

// slowing hazards
bool is_in_slowing_hazards(float playerX, float playerY, float playerRadius) {
	bool isInHazard = false; // Flag to track if the player is in any hazard

	// this function is called twice per frame (flight and crawl), so only advance and draw the
	// honey/cobweb messages on the first call each frame, otherwise they last half as long
	static unsigned int lastMessageFrame = 0;
	bool firstCallThisFrame = (CP_System_GetFrameCount() != lastMessageFrame);
	lastMessageFrame = CP_System_GetFrameCount();

	// Check cobwebs
	for (int i = 0; i < MAX_COBWEBS_HAZARDS; i++) {
		float dx = playerX - cobwebHazards[i].x;
		float dy = playerY - cobwebHazards[i].y;
		float distance = sqrtf(dx * dx + dy * dy);

		// using just the larger of the two radii
		float collisionRadius = (cobwebHazards[i].radius > playerRadius) ?
			cobwebHazards[i].radius : playerRadius;

		if (distance < collisionRadius) {
			isInHazard = true;
			cobwebShowMessage = true;
			cobwebMessageDuration = 0.0f;
		}
	}

	// Handle message display (outside the cobweb loop, so the timer only advances once)
	if (cobwebShowMessage && firstCallThisFrame) {
		cobwebMessageDuration += CP_System_GetDt();
		if (cobwebMessageDuration < 1.0f) {
			CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
			CP_Settings_TextSize(FONT_SIZE_BODY);
			CP_Font_DrawText("Can't move fast in this cobweb...", (float)CP_System_GetWindowWidth() / 2, 270.0f);
		}
		else {
			cobwebShowMessage = false;
		}
	}

	// Check honey (only the honey hazards placed for the current mode)
	for (int i = 0; i < activeHoneyHazards; i++) {
		float dx = playerX - honeyHazards[i].x;
		float dy = playerY - honeyHazards[i].y;
		float distance = sqrtf(dx * dx + dy * dy);

		if (distance < (playerRadius + honeyHazards[i].radius)) {
			isInHazard = true;
			honeyShowMessage = true;
			honeyMessageDuration = 0.0f;
		}
	}

	// Handle message display
	if (honeyShowMessage && firstCallThisFrame) {
		honeyMessageDuration += CP_System_GetDt();
		if (honeyMessageDuration < 1.0f) {
			CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
			CP_Settings_TextSize(FONT_SIZE_BODY);
			CP_Font_DrawText("Can't move fast in this sticky situation...", (float)CP_System_GetWindowWidth() / 2, 270.0f);
		}
		else {
			honeyShowMessage = false;
		}
	}

	return isInHazard;
}

float get_hazard_speed(float originalSpeed) {
	return originalSpeed * 0.1f;
}

float get_hazard_fall_speed(float verticalVelocity) {
	if (verticalVelocity > 0) {
		if (verticalVelocity < 10.0f) {
			verticalVelocity = 10.0f;
		}
		if (verticalVelocity > 2.0f) {
			verticalVelocity = 2.0f;
		}
	}
	return verticalVelocity;
}

// flying
void player_flight() {
	float deltaTime = CP_System_GetDt();
	float horizontalSpeed = player.speed * deltaTime * 0.5f;  // Reduced speed multiplier for smoother movement
	float verticalJumpSpeed = player.speed * deltaTime * 6.0f; // Adjusted for better jump height

	// checks if player is touching any slowing hazard while in the air
	bool inSlowingHazard = is_in_slowing_hazards(player.x, player.y, PLAYER_RADIUS);
	if (inSlowingHazard) {
		horizontalSpeed = get_hazard_speed(horizontalSpeed);
		verticalJumpSpeed *= 0.15f;

		// Only modify fall speed, not upward movement
		if (verticalVelocity > 0) {
			verticalVelocity = get_hazard_fall_speed(verticalVelocity);
		}
	}

	mate_player_collision();
	// Track ground state
	bool isGrounded = checkCollision(player.hitbox, groundHitboxUpper);
	bool isOnPlatform = false;

	// Check for platform collision
	for (int i = 0; i < MAX_PLATFORMS; i++) {
		if (checkCollision(player.hitbox, platforms[i].hitbox)) {
			isOnPlatform = true;
			// player.y = platforms[i].y - PLAYER_RADIUS;  // Adjust y to platform top
			// verticalVelocity = 0.0f;                    // Reset velocity to prevent falling
			break;
		}
	}

	// If on ground or platform, reset vertical velocity
	if ((isGrounded || isOnPlatform) && verticalVelocity >= 0) {
		verticalVelocity = 0.0f;
		player.y = isGrounded ? upperGroundTop - PLAYER_RADIUS : player.y; // Adjust y if grounded
	}
	else {
		// Apply gravity when in the air
		verticalVelocity += 500.0f * deltaTime;
	}

	// ------------------------------- MOVEMENT ----------------------------------
	if (!isOutOfStamina && currentStamina > maxStamina / 5) {
		// Fly leftwards
		if (CP_Input_KeyTriggered('A')) {
			player.x -= horizontalSpeed;
			verticalVelocity = -verticalJumpSpeed;
			facingRight = false;

			currentStamina -= 5.0f; // Deplete stamina
		}

		// Fly rightwards
		if (CP_Input_KeyTriggered('D')) {
			player.x += horizontalSpeed;
			verticalVelocity = -verticalJumpSpeed;
			facingRight = true;

			currentStamina -= 5.0f; // Deplete stamina
		}

		// Fly upwards
		if (CP_Input_KeyDown('A') && CP_Input_KeyDown('D')) {
			verticalVelocity = -verticalJumpSpeed;
			currentStamina -= staminaDepletionRate * deltaTime; // Continuous stamina depletion for flying
		}
	}

	// ----------------------------------- DASH -----------------------------------
	// Dash leftwards
	if (CP_Input_KeyTriggered('A') && CP_Input_KeyDown(KEY_SPACE)) {
		if (!isOutOfStamina && currentStamina > maxStamina / 5) {
			float dashSpeed = player.speed * deltaTime * 2.0f;
			player.x -= dashSpeed;
			verticalVelocity = -verticalJumpSpeed;
			facingRight = false;

			onCooldown = true;
			dashCooldownTimer = dashCooldownDuration;
			currentStamina -= 10.0f;
		}
	}

	// Dash rightwards
	if (CP_Input_KeyTriggered('D') && CP_Input_KeyDown(KEY_SPACE)) {
		if (!isOutOfStamina && currentStamina > maxStamina / 5) {
			float dashSpeed = player.speed * deltaTime * 2.0f;
			player.x += dashSpeed;
			verticalVelocity = -verticalJumpSpeed;
			facingRight = true;

			onCooldown = true;
			dashCooldownTimer = dashCooldownDuration;
			currentStamina -= 10.0f;
		}
	}

	// Apply vertical velocity to simulate upward and downward movement
	player.y += verticalVelocity * deltaTime;

	// Update hitbox position to match player position
	player.hitbox.x = player.x;
	player.hitbox.y = player.y;
}

// crawling
void player_crawl() {
	float deltaTime = CP_System_GetDt();
	float velocity = 150.0f;
	float speed = velocity * deltaTime;
	float mousePosition = CP_Input_GetMouseX();

	// Transform mouse position to world space to account for vision system zoom and translation
	float zoomFactor = visionSystem.zoomFactor;
	float windowWidth = (float)CP_System_GetWindowWidth();
	float windowHeight = (float)CP_System_GetWindowHeight();

	// transform mouse position
	float relativeMouse = (mousePosition - (windowWidth / 2.0f)); // mousePosition (screen space) & windowWidth/2 centers the mouse relative to the middle of the screen.
	float worldSpace = relativeMouse / zoomFactor;  // divides by zoom factor to scale the coordinates back into world space ("returning"them to their original positions in the world)
	float transformedMouseX = worldSpace + player.x; // reposition player's world position.
	float transformedPlayerX = player.x;

	// Check if player is on the ground
	bool isGrounded = checkCollision(player.hitbox, groundHitboxUpper);
	bool isOnPlatform = false;

	// Check for platform collision (if player is on platform)
	for (int i = 0; i < MAX_PLATFORMS; i++) {
		if (checkCollision(player.hitbox, platforms[i].hitbox)) {
			isOnPlatform = true;
			player.y = platforms[i].hitbox.y - PLAYER_RADIUS;
			break;
		}
	}

	// Check for collision with sticky sap
	bool inStickySap = is_in_slowing_hazards(player.x, player.y, PLAYER_RADIUS);
	if (inStickySap) {
		speed *= 0.5f; // Slow down crawl speed by half
	}


	if (isGrounded || isOnPlatform) {
		if (isGrounded) {
			player.y = upperGroundTop - PLAYER_RADIUS;
		}

		// Move based on the transformed mouse position
		if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT)) {
			if (transformedMouseX < transformedPlayerX) {
				player.x -= speed;
				facingRight = false;
			}
			else if (transformedMouseX > transformedPlayerX) {
				player.x += speed;
				facingRight = true;
			}
		}

		// ----------------------------------- DASH -----------------------------------
		if (!isOutOfStamina && !inStickySap && (CP_Input_MouseDown(MOUSE_BUTTON_LEFT) && CP_Input_KeyDown(KEY_SPACE))) {
			float dashSpeed = 200.0f * deltaTime;
			if (transformedMouseX < transformedPlayerX) {
				player.x -= dashSpeed;
				facingRight = false;
			}
			else if (transformedMouseX > transformedPlayerX) {
				player.x += dashSpeed;
				facingRight = true;
			}
		}

		// Update hitbox position
		player.hitbox.x = player.x;
		player.hitbox.y = player.y;
	}
}

void dashCooldown() {
	float deltaTime = CP_System_GetDt();
	bool playerFlight = CP_Input_KeyDown('A') || CP_Input_KeyDown('D');
	bool playerDash = playerFlight && CP_Input_KeyDown(KEY_SPACE);

	// if dash on cooldown, update timer
	if (onCooldown) {
		dashCooldownTimer -= deltaTime;
		if (dashCooldownTimer < 0.0f) {
			onCooldown = false;
			dashCooldownTimer = 0.0f;
		}
	}

	// check if player dashed
	if (playerDash || playerFlight) {
		// if dashed, start 3 sec countdown
		if (!onCooldown && !isOutOfStamina) {
			onCooldown = true;
			dashCooldownTimer = dashCooldownDuration;
		}
		else {
			/*printf("Dash is on cooldown! Time remaining: %.2f seconds\n", dashCooldownTimer);*/
		}
	}
}

void stamina() {
	float deltaTime = CP_System_GetDt();

	// Calculate stamina depletion if flying
	bool isFlying = CP_Input_KeyDown('A') || CP_Input_KeyDown('D');
	bool isDashing = isFlying && CP_Input_KeyDown(KEY_SPACE);
	bool isFlyingUp = CP_Input_KeyDown('A') && CP_Input_KeyDown('D');

	// Check for crawl dashing
	bool isCrawlDashing = CP_Input_MouseDown(MOUSE_BUTTON_LEFT) && CP_Input_KeyDown(KEY_SPACE);

	// Deplete stamina for flying and dashing
	if (isFlying || isFlyingUp) {
		currentStamina -= 10.0f * deltaTime;
	}
	if (isDashing || isCrawlDashing) {
		currentStamina -= staminaDepletionRate * deltaTime;
	}

	// debugging
	//printf("current stamina: %f \n", currentStamina);

	// Ensure stamina does not go below 0
	if (currentStamina < 0.0f) {
		currentStamina = 0.0f;
		isOutOfStamina = true;
	}
	else {
		isOutOfStamina = false;
	}
	// check if player is on the ground
	bool isGrounded = checkCollision(player.hitbox, groundHitboxUpper);
	bool isOnPlatform = false;

	// Check for platform collision (if player is on platform)
	for (int i = 0; i < MAX_PLATFORMS; i++) {
		if (checkCollision(player.hitbox, platforms[i].hitbox)) {
			isOnPlatform = true;
			player.y = platforms[i].y - PLAYER_RADIUS;  // Adjust y to platform top
			verticalVelocity = 0.0f;
			break;
		}
	}

	if (isOutOfStamina && !isGrounded && !isOnPlatform) {
		verticalVelocity += 500.0f * deltaTime;
		player.y += verticalVelocity * deltaTime;
	}

	// Automatically regenerate stamina if not moving/when flying
	if (!onCooldown && (currentStamina == 0.0f || currentStamina < maxStamina) && !CP_Input_KeyDown(KEY_SPACE) && !isCrawlDashing) {
		currentStamina += 25.0f * deltaTime;
		if (currentStamina > maxStamina) {
			currentStamina = maxStamina;
		}
	}

	if ((!isDashing && !isCrawlDashing && !isFlyingUp) && currentStamina < maxStamina) {
		currentStamina += 25.0f * deltaTime;
		if (currentStamina > maxStamina) {
			currentStamina = maxStamina;
		}
	}

	// Update stamina bar width based on current stamina
	staminaBarWidth = 205.0f * (currentStamina / maxStamina);
}

void drawStaminaBar() {
	// Draw the stamina bar and triangle to show depletion
	staminaBarX = 90.0f;
	staminaBarY = 63.0f;
	staminaBarHeight = 25.0f;
	staminaBarColor = CP_Color_Create(245, 178, 2, 255);
	CP_Settings_Fill(staminaBarColor);
	CP_Settings_StrokeWeight(0.0f);
	CP_Graphics_DrawRect(staminaBarX, staminaBarY, staminaBarWidth, staminaBarHeight);

	// Update triangle position based on new stamina bar width
	staminaTip1X = staminaBarX + staminaBarWidth;          // right edge of the rectangle
	staminaTip1Y = staminaBarY;                            // top of the bar
	staminaTip2X = staminaTip1X + 13.0f;                   // extend to the right for the trianle tip
	staminaTip2Y = staminaBarY + (staminaBarHeight / 2);   // middle of the bar
	staminaTip3X = staminaTip1X;                           // right edge of the rectangle
	staminaTip3Y = staminaBarY + staminaBarHeight;         // bottom of the bar
	CP_Graphics_DrawTriangle(staminaTip1X, staminaTip1Y, staminaTip2X, staminaTip2Y, staminaTip3X, staminaTip3Y);

	// Set image drawing settings
	CP_Settings_ImageMode(CP_POSITION_CORNER);

	// Draw stamina bar image
	CP_Image_Draw(staminaBar, 40.0f, 15.0f, 300.0f, 300.0f, 255);
}

void health() {
	// to check if falling:
	// check if the player is on the ground or on the platforms
	// check if the player is pressing any movement keys
	// if all NO, player is in a falling state

	bool isOnPlatformHealth = false;
	bool isGroundedHealth = checkCollision(player.hitbox, groundHitboxUpper);
	float fallDistance = 0.0f;


	// Check if the player is on any platform
	for (int i = 0; i < MAX_PLATFORMS; i++) {
		if (checkCollision(player.hitbox, platforms[i].hitbox)) {
			isOnPlatformHealth = true;
			break;
		}
	}

	// Determine if the player is falling (not grounded, on a platform, or jumping)
	if (!(CP_Input_KeyDown('A') || CP_Input_KeyDown('D'))) {
		if (!isGroundedHealth && !isOnPlatformHealth) {
			isFallingHealth = true;
			isLandingHealth = false;

			// Handle the start of a fall
			if (!fallStartedHealth) {
				initialYHealth = player.y; // Record the starting Y position of the fall
				fallStartedHealth = true; // Mark that the fall has started
				//printf("Player initial Y Position: %.2f\n", initialYHealth); // Print once at the start of the fall
			}
		}
		else if (isGroundedHealth || isOnPlatformHealth) {
			isLandingHealth = true;

			// Handle landing logic
			if (isFallingHealth) {
				isFallingHealth = false; // Once landed, set isFalling to false
				finalYHealth = player.y; // Record the Y position where the player lands
				/*printf("Player has landed! Final Y: %.2f\n", finalYHealth);*/

				// Calculate fall distance and apply damage
				float fallDistance = finalYHealth - initialYHealth;
				if (fallDistance >= 225.0f) {
					/*printf("Player fell from a height of %.2f!\n", fallDistance);
					printf("Player lost 1 life!\n");*/
					if (!gameRestarted) {
						CP_Sound_Play(fallDamage);
						player.health -= 1.0f;
					}
				}

				// Reset fall variables for the next fall
				fallStartedHealth = false;
				initialYHealth = player.y; // Reset for future falls
			}
		}
	}

	// Handle jump input
	if (CP_Input_KeyDown('A') || CP_Input_KeyDown('D')) {
		if (isFallingHealth) {
			// Player initiated a jump while falling
			initialYHealth = player.y; // Reset to the current position for the new jump
			fallStartedHealth = false; // Mark that this is a new jump, not a continuation of the fall
			//printf("Player initial Y Position: %.2f\n", initialYHealth); // Print once at the start of the fall
		}
	}

	float healthIcon1X = 75.0f;
	float healthIcon1Y = 80.0f;
	float healthIcon2X = 115.0f;
	float healthIcon2Y = 80.0f;
	float healthIcon3X = 155.0f;
	float healthIcon3Y = 80.0f;
	float healthIcon4X = 195.0f;
	float healthIcon4Y = 80.0f;
	float healthIcon5X = 235.0f;
	float healthIcon5Y = 80.0f;

	if (player.health == 5.0f) {
		CP_Image_Draw(heartIcon, healthIcon1X, healthIcon1Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon2X, healthIcon2Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon3X, healthIcon3Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon4X, healthIcon4Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon5X, healthIcon5Y, 80.0f, 80.0f, 255);
	}
	else if (player.health == 4.0f) {
		CP_Image_Draw(heartIcon, healthIcon1X, healthIcon1Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon2X, healthIcon2Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon3X, healthIcon3Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon4X, healthIcon4Y, 80.0f, 80.0f, 255);
	}
	else if (player.health == 3.0f) {
		CP_Image_Draw(heartIcon, healthIcon1X, healthIcon1Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon2X, healthIcon2Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon3X, healthIcon3Y, 80.0f, 80.0f, 255);
	}
	else if (player.health == 2.0f) {
		CP_Image_Draw(heartIcon, healthIcon1X, healthIcon1Y, 80.0f, 80.0f, 255);
		CP_Image_Draw(heartIcon, healthIcon2X, healthIcon2Y, 80.0f, 80.0f, 255);
	}
	else if (player.health == 1.0f) {
		CP_Image_Draw(heartIcon, healthIcon1X, healthIcon1Y, 80.0f, 80.0f, 255);
	}
}

void guide() {
	float arrowIconX = (float)CP_System_GetWindowWidth() / 2, arrowIconY = (float)CP_System_GetWindowHeight();
	float arrowAngle = 0;

	// if player's y pos is below the mate's y pos
	if (player.y > mate.y) {
		// if player's x pos is to the left of the mate
		if (player.x < mate.x) {
			arrowIconX = CP_System_GetWindowWidth() - 200.0f;
			arrowIconY = 0.0f + 100.0f;
			arrowAngle = -125;

		}
		else {
			arrowIconX = (float)CP_System_GetWindowWidth() / 2;
			arrowIconY = 0.0f + 100.0f;
			arrowAngle = 180;
		}
	}
	// if player's y pos is above the mate's y pos
	else if (player.y < mate.y) {
		// if player's x pos is to the left of the mate
		if (player.x < mate.x) {
			arrowIconX = CP_System_GetWindowWidth() - 200.0f;
			arrowIconY = CP_System_GetWindowHeight() - 200.0f;
			arrowAngle = -45;
		}
		else {
			arrowIconX = (float)CP_System_GetWindowWidth() / 2;
			arrowIconY = CP_System_GetWindowHeight() - 200.0f;
			arrowAngle = 0;
		}
	}

	CP_Image_DrawAdvanced(arrowIcon, arrowIconX, arrowIconY, 60.0f, 60.0f, 255, arrowAngle);
}
 
void mate_player_collision() {
	if (currentState != STATE_GAME_COMPLETED && checkCollision(player.hitbox, mate.hitbox)) {
		CP_Sound_StopGroup(CP_SOUND_GROUP_MUSIC);
		CP_Sound_Play(gameComplete);
		//CP_Engine_SetNextGameState(game_complete_init, game_complete_update, game_complete_exit);
		currentState = STATE_GAME_COMPLETED;
		//printf("GAME COMPLETED\n");
	}
}
void player_draw(void) {
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	float drawWidth = facingRight ? SPRITE_SCALE * 1.5f : -SPRITE_SCALE * 1.5f;
	CP_Image_Draw(playerSpriteFrames[currentFrame], player.x, player.y, drawWidth, (float)(SPRITE_SCALE * 1.5), 255); // Draw player sprite
}

void mate_draw(void) {
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	float drawWidth = -SPRITE_SCALE * 1.5f;
	CP_Settings_Fill(CP_Color_Create(237, 227, 176, 100));
	CP_Settings_Stroke(CP_Color_Create(237, 227, 176, 255));
	CP_Graphics_DrawCircle(mate.x, mate.y, 100.0f);
	CP_Image_Draw(mateSprite, mate.x, mate.y, drawWidth, (float)(SPRITE_SCALE * 1.5), 255); // Draw Mate sprite
}

void player_exit(void) {
	for (int i = 0; i < 2; i++) {
		CP_Image_Free(&playerSpriteFrames[i]);
	}

	// free everything else loaded in player_init
	CP_Image_Free(&mateSprite);
	CP_Image_Free(&staminaBar);
	CP_Image_Free(&heartIcon);
	CP_Image_Free(&arrowIcon);
	CP_Sound_Free(&fallDamage);
	CP_Sound_Free(&playerCollision);
	CP_Sound_Free(&gameComplete);
	CP_Sound_Free(&gameOverSound);
}
