Comments to TA about your implementation (optional):
- Examples: known bugs or incomplete implementation

# CPSC 427 Farmer-Defense - The Last Days

**Group**

- Kung Lim
- Daniel Lee
- Anna Tao
- Ziqing Wang
- Haonan Zhang

**Video**

- []

**Description**

- []

**Feature**

## M1 stage:

* **Player Movement**: Players can move within a defined game space (lawn area) using "WASD" keys(well-defined game-space boundaries).

* **Combat System**: Players can attack by clicking the left mouse button, featuring simple collision detection & resolution.

* **Game Controls**: Players can restart the game by pressing "R" and exit the game by pressing "Esc".

* **Enemy System**: Zombies spawn in waves and continuously chase (Basic 2D transformations) and attack the player (Random/coded action).

* **Manual Wave Spawn**: Players can manually trigger zombie waves by pressing "G".

* **Experience System**: Players gain experience from killing zombies, which are displayed with an experience bar.

* **Health System**: When the player is attacked, they will lose a certain amount of health. If their health reaches zero, they will be eliminated. This is displayed with an health bar.

* **Game Progression**: Players can continue playing until they want to end or are eliminated.

* **Game Over State**: Upon player death, a "Game Over" screen appears and all entities in the game freeze.

* **Visual Assets**: Both players and zombies feature unique textures and animations.

* **Hit Effects**: Both players and zombies have special effects when taking damage.

* **Audio Features**: The game includes background music and sound effects for player movement, player attacks, player damage, and enemy damage.

* **User Interface**: The UI displays player health, experience points, inventory, and a pause button.

* **Console Counter**: The counter counts the number of alive zombies left in the game and the number of zombies you killed in this round and prints it on console.



**Setup**

- []

**Citation**

- Grass texture by p0ss sourced from https://opengameart.org/node/9692
- Dirt texture by p0ss (farmland) sourced from https://opengameart.org/node/9021
- Dirt texture by Lamoot (scorched earth) sourced from https://opengameart.org/node/7805
- Seeds sourced from _'Crops'_ by SnoopethDuckDuck: https://snoopethduckduck.itch.io/crops
- https://pixabay.com/sound-effects/

[comment]: <> (* **Scoring System**: Players earn points for killing zombies, which are displayed on the scoreboard.)
