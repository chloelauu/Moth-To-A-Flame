# Moth To A Flame

A 2D platformer in C where a small blue moth crosses a dark forest to find its mate before its light runs out.

**Screenshots and gameplay video:** https://chloe-gportfolio.netlify.app/projects/moth-to-a-flame

## About the Game

You play a moth searching a shadowy forest for its mate. Fly, dash and crawl past frogs, birds, fire, beehives and sticky hazards, collect food to restore your health, and rest at lamps and glowing mushrooms to recharge your light.

There are two modes. In Easy mode you can see the whole forest. In Hard mode you can only see what your light reaches, and the game ends if it runs out.

A team of four, **Team Skibidi**, made it in C on DigiPen's CProcessing library for Software Engineering Project 1 (CSD1401) at the Singapore Institute of Technology (DigiPen), September – November 2024.

## My Role

During the project, I worked on game state management, the camera, and parts of the player's movement and collisions.

- **Camera (C):** made the camera follow the player, with a slight delay (lerp) for smoother movement.
- **Movement and collisions (C):** worked on velocity-based movement, collision with the map and platforms, and the player's game object.
- **Hazards (C):** built the sticky hazards (honey and cobwebs) that slow the player down.
- **Game flow (C):** built the game state management, the game over screen, the stamina recharge, and the Easy and Hard modes.

### Polish after the course (2026)

After the course, I fixed bugs and polished the menus. The gameplay and art are the team's original work.

- Consistent fonts and animated buttons on every screen, plus a redesigned main menu and Controls screen
- Animated backgrounds for the game over and credits screens, and a new pause screen
- Fixes for restarting, memory leaks and sounds overlapping between screens
- Windowed mode by default, with F11 for fullscreen

## Download and play

1. Download **MothToAFlame-Windows.zip** from the [latest release](../../releases/latest).
2. Unzip it and run **MothToAFlame.exe**.

Windows only. If Windows shows "Windows protected your PC", click **More info → Run anyway**.
If the game says a file like `VCRUNTIME140.dll` is missing, install the [Microsoft Visual C++ Redistributable (x64)](https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist).

## Controls

| Action | Control |
|---|---|
| Fly up-left / up-right | A / D |
| Fly straight up | A + D |
| Dash | Hold Space, then tap A or D |
| Crawl toward the cursor | Hold Left Click and aim with the mouse |
| Crawl faster | Left Click + Space |
| Pause | Esc |
| Back (Guide, Controls, Mode Select, Credits) | Esc or the Back button |
| Restart / main menu / quit (game over) | R / M / Q |
| Fullscreen / windowed | F11 (fullscreen changes your display resolution) |

## Team

- **Chloe Lau Rey En:** game state management, camera, sticky hazards, game over screen, difficulty modes
- **Tan Shi Qi Charity:** player movement and stamina, map design, credits screen, moth art
- **Tan Jia Qi Trish:** bird enemy, food, health and damage, damage hazards, visual cues, enemy and light-source art
- **Choi Meng Yew:** frog enemy, main menu, guide and controls screens, safe zones, audio

## Asset credits

- Fonts: Darkening (1001 Fonts), Wicked Scary Movie

## Building from source

The project is a Visual Studio solution (C, x64). It needs DigiPen's CProcessing library (with FMOD), which is not included in this repository. Place it in `Extern/CProcessing/` to build.

## Copyright

All content © 2024 DigiPen Institute of Technology Singapore. All rights reserved.
