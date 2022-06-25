# Simple game: race to the star
### Collisions, gameloop, level loading from file

1. two players: square and circle
2. **steering**:
   1. square - `keyboard`: `wsad` / `arrows`
   2. circle - `gamepad`
3. each player model has their own collision resolution type
4. collision resolution type:
   1. with walls - **post-collision**
   2. with screen bounds - **pre-collision**
5. arrow at the bottom points to the target (only if it's not already visible)
6. there are `3` levels with increasing difficulty
7. **scoring**:
   1. the first player to reach the star earns a point
   2. the player with the most points after **3 rounds** wins

![Simple game simulation](visualisation.gif)