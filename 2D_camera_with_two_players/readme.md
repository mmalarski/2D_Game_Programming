# 2D Camera with two players with level map loading from file

1. there are two players
   1. one steering with `keyboard`: `wsad` / `arrows`
   2. the other with `gamepad`
   3. there's active gamepad detection for the second player - if no gamepad is connected only the first player will be active
2. (with two players) you can switch the camera focus with `c` keyboard button between three modes:
   1. focus on both players
   2. focus only on the first player
   3. focus only on the second player
3. while the camera is focused on one player the other's input is turned off

![Level with two players and camera focused on both of them](visualisation.gif)