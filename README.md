# CPSC 427 Farmer-Defense - The Last Days

**Group**

- Kung Lim
- Daniel Lee
- Anna Tao
- Ziqing Wang
- Haonan Zhang

**Video**

- https://youtu.be/HwS0V2wAJzY

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
- **Left Click** – Attack  
- **G** – Spawn enemy  
- **R** – Restart game  
- **ESC** – Close game 

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