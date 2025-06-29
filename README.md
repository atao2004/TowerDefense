# CPSC 427 Farmer-Defense - The Last Days

**Group**

- Kung Lim
- Daniel Lee
- Anna Tao
- Ziqing Wang
- Haonan Zhang

**Video**

- https://youtu.be/HwS0V2wAJzY
- https://youtu.be/OYWhfxRYuS4
- https://www.youtube.com/watch?v=d-8PPzW4rto

**Description**

- This is a 2D survival game where players control a farmer while fending off endless waves of zombies.

**Citation**

- Dirt texture by Lamoot (scorched earth) sourced from https://opengameart.org/node/7805
- Seeds sourced from _'Crops'_ by SnoopethDuckDuck: https://snoopethduckduck.itch.io/crops
- https://pixabay.com/sound-effects/
- combat/night bgms from https://www.gamedevmarket.net/asset/celtic-countryside-music
- game_over sound effect from https://pixabay.com/sound-effects/search/game-over/
- attack effect from https://frostwindz.itch.io/pixel-art-slashes
- zombie and player characters from https://kenney.nl/assets/platformer-characters
- gameover png https://stock.adobe.com/search?k=%22game+over%22&asset_id=244026851

[comment]: <> (* **Scoring System**: Players earn points for killing zombies, which are displayed on the scoreboard.)

## Controls:

- **WASD** – Move the player  
- **Left Click** – Attack/click button/choose seeds in leveling up screen
- **Right Click (or F)** - Plant seed
- **C** - Summon chicken
- **G** – Spawn enemy
- **R** – Restart game
- **T** - Enter/Exit tutorial  
- **ESC** – Close game 
- **"SPACE"** – Dash
-  **"number key 1~7"**  – Choose seed in the inventory

## M1 Features:

**All**

- **Player Movement**: Player can move within a defined game space (lawn area) using "WASD" keys (well-defined game-space boundaries).

- **Combat System**: Players can attack by clicking the left mouse button, featuring simple collision detection & resolution.

- **Game Controls**: Players can restart the game by pressing "R" and exit the game by pressing "Esc".

- **Enemy System**: Zombies spawn in waves and continuously chase (Basic 2D transformations) and attack the player (Random/coded action).

- **Manual Wave Spawn**: Players can manually trigger zombie waves by pressing "G".

- **Experience System**: Players gain experience from killing zombies, which are displayed with an experience bar.

- **Health System**: When the player is attacked, they will lose a certain amount of health. If their health reaches zero, they will be eliminated. This is displayed with a health bar.

- **Game Progression**: Players can continue playing until they want to end or are eliminated.

- **Game Over State**: Upon player death, a "Game Over" screen appears and all entities in the game freeze.

- **Visual Assets**: Both players and zombies feature unique textures and animations.

- **Hit Effects**: Both players and zombies have special effects when taking damage.

- **Audio Features**: The game includes background music and sound effects for player movement, player attacks, player damage, and enemy damage. Background music will switch between 2 soundtracks when player is in combat and out of combat.

- **User Interface**: The UI displays player health, experience points, inventory (not implemented), and a pause button (not implemented).

- **Console Counter**: The counter counts the number of alive zombies left in the game and the number of zombies you killed in this round and prints it on console.

**Required [3] Rendering: Key-frame/state interpolation**

- The game over screen fades out to black. This uses linear interpolation implemented in vignette.fs.glsl.

**Creative [1] Graphics: Simple rendering effects**

- The player when taking damage flashes red.
- The enemy when taking damage flashes white.
- The enemy when killed is knocked back while fading out.

**Creative [23] User Interface: Audio feedback**
- There are two background tracks depending on whether the player is in combat (there is at least 1 zombie).
- There are sound effects for player move, player attack, enemy attack, player death, and game over.




## M2 Features:

### Required Elements

**Improved Game AI and Gameplay**
* Skeleton
   * The skeleton is a new enemy with two states (walk, attack). In the walk state, the enemy moves toward the player and plants. When they are in range, the enemy will transition to the attack state and fire an arrow. Regular zombies are unable to harm plants.
* Plant
   * Press "f" to plant a seed in farmland. After a certain amount of time, seeds will mature into plants with two states (idle, attack). In the idle state, if there is an enemy on the map, the seed will transition to the attack state and fire a projectile. The player can obtain new seeds from levelling up

**Sprite Animation**
* Player (Idle, Move, Attack)
* Zombie (Move)
* Skeleton (Idle, Walk, Attack)
* Plant (Attack)
* Tutorial Signs

**New Assets**
* Map (Grass, Farmland, Rocks, Flowers, Trees)
* Refer to Sprite Animation assets.

**Mesh-based Collision Detection & Resolution**
* Press "C" to summon a giant chicken. The chicken will kill any enemies in its path using mesh-based collisions.
* The chicken will also be summoned when player's health is less that 1/4

**Gameplay Tutorial**
* Press "T" to swap between the playing and tutorial map. 
* The tutorial map has 4 instruction boards and 2 tutorial enemies to help player get started with the game.

**FPS Counter**
* The FPS is printed on the game window.

**Non-Repetitive Gameplay**
* The number of enemies in each wave gradually increases, making survival more challenging over time. To counter this, players can level up by defeating enemies. Each level-up rewards players with seeds, which can be used to grow plants for defense.

**Inventory(Incomplete)**
* Inventory on the bottom of screen can show the number of seeds owned by the player

### Creative Elements

**[21] User Interface: Camera controls**
* The camera follows the player.

**[24] Quality & User Experience (UX): Basic integrated assets**
* Refer to Sprite Animation and New Assets
* The game map and tutorial map was redesigned using a tileset.


## M3 Features:

### Basic Improvement
* The game now has a splash screen where players can choose to start a new game, load a saved game, play the tutorial, or exit(currently need to reopen the project to show the splash screen)
* The game now displays FPS, current enemy count, and current plant count on the screen
* The game now has a more logically arranged map
* The game now has more types of enemies with different appearances, health values, speeds, and attacks
* Inventory now shows the seed count
* The game has been optimized to improve frame rate
* Adjusted the difficulty curve for a better gaming experience

### Creative Elements

* **Basic [27] Story Elements**: The game now introduces background story when the game starts, and a cutscene appears when the player's first planted seed matures, and there is a cutscene when player first level up
* **Basic [19] Reloadability**: Players can now save and load their game. Press button "-" to load the game(or click on "Load Game" in splash screen) and press "=" to save the game(cannot save the game when the chicken is summoned, or during the cutscene)
* **Advanced [5] Particle System**: The game features an instanced rendering particle system that supports importing images as particle textures or using default shapes. Currently implemented particle effects include:
  * Blood effects when player hits enemies
  * Growth effects when seeds mature
  * Level-up effects when player gains a level
  *(Note: Particle effects may not work on all computers. If you find that particle effects are not rendering properly, please contact us)*



## M4 Features:

### Basic Improvement
* Added 1 new enemy type:The Orc Knights with a new charging attack pattern
* Added 6 new tower types:
  * Two towers that can fire projectiles with different speed and damage
  * One tower that can heal the player within its range
  * One tower that damages enemies within its range
  * One tower that slows enemy movement speed within its range
  * One tower that generates electrical current between two towers
* Added 6 new seed types that grow into corresponding towers. Players can select them from the inventory using number keys
* Added a pause menu where players can load game, save game, or exit game
* Added a level-up reward menu. When leveling up, players can choose a seed as a reward from the menu
* Enhanced UI with more game information
* The game over screen now shows the number of enemies killed, days survived, and final level of the player.
* The enemies have been specified for days 1-11.- Random enemies for day 12 and onwards.
* Small fade effect to mark the transition from one day to the next.
* Player can attack up/down and dash now.

### Creative Elements
* **Challenge Level**: On the day 5, tactical squad enemies will appear who cooperate to attack the player
* **Game Balance**: Adjusted game balance to improve players' flow