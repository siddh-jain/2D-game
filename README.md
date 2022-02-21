# 2D Game

## About the Application
- The game is built in C++ using openGL platform.
- The game is a top-down one with fixed frame dungeon rooms.

## Description of Project
- The game world consists of 3 rooms, where each room has a door at the edge. The player's objective is to reach the final gate without being destroyed by the enemies.
- Every room contains of randomly generated walls (apart from edge walls).
- Every room contains of randomly spawned coins and enemies.
- Player's points increase on collecting coins whereas the plyer loses on coming in contact with the moving/ static enemies.
- Difficulty of the game increases with progress.
- 'L' key can be used to toggle the lights. When the lights are on, only some area surrounding the player will be visible while the rest of the world will go dark. However, playing in dark will give more points to the player.
- for more details refer to [this doc](Escape-man.pdf)

## Game Render

Move to game root directory

    mkdir build
    cd build
    cmake ..
    make -j
    ./app    

## Game Controls

Key bindings
- Up- move player up
- Down- move player down
- Left- move player left
- Right- move player right
- L : Lights toggle

