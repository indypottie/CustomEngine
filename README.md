# Bomberman Game Engine Project

## Overview
This project is a custom game engine built in C++ designed specifically for creating a Bomberman-style game. It supports multiple levels, AI enemies, player controls, bomb mechanics, and a HUD system. The engine emphasizes modularity, extensibility, and maintainability to facilitate easy feature additions and game logic enhancements.

## Key Features
- **Modular Component System:**  
  Entities in the game are built using a component-based architecture. Components like `ColliderComponent`, `PlayerComponent`, and `EnemyComponent` enable flexible behavior composition and separation of concerns.

- **Grid-Based Level Management:**  
  Levels are loaded from JSON files describing the tile grid, walkable areas, and spawn points. The grid supports navigation, enemy spawning with constraints, and interaction with game objects like exits and bombs.

- **Input Handling:**  
  Supports both keyboard and gamepad input with a command pattern. Commands are bound dynamically to input events for player movement and bomb actions.

- **AI and Enemy Behavior:**  
  Enemies are spawned at random walkable positions excluding the initial player area. They have simple AI behaviors and are managed centrally by the `LevelManager`.

- **Collision System:**  
  Uses a `ColliderComponent` to handle collision detection. Colliders update their positions relative to their game object’s world position each frame to ensure accurate collision handling.

- **Level Progression:**  
  The game supports multiple levels (3 total) with logic to advance to the next level once all enemies are defeated and the player drops a bomb on the exit tile. A skip-level feature is implemented for testing or player convenience.

- **HUD System:**  
  Displays score and remaining enemies dynamically, updating based on game events.

## Design Choices

### Component-Based Architecture
Choosing a component-based design allowed decoupling of functionalities and promoted reusability. This makes it easier to add or modify features without altering the entire entity system.

### Data-Driven Level Loading
Using JSON files for level data enables easy editing and expansion of game levels without recompiling the engine. The grid system abstracts tile management and world coordinates for seamless interaction.

### Command Pattern for Input
Implementing the command pattern for input decouples input handling from game logic. This makes it simple to rebind keys or support different input devices with minimal changes.

### Sound and Resource Management
Sound playback and resource loading are centralized via a Service Locator pattern, which simplifies managing global systems and facilitates swapping implementations if needed.

### Multilevel Management and Progression
The `LevelManager` handles enemy tracking, player progress, and level transitions, encapsulating game state logic to keep game flow management organized and clear.


https://github.com/indypottie/Prog4-Exam-Bomberman.git