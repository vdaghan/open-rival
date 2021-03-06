# Game Mechanics

## Rendering

 - The game is rendered using the "zig-zag" approach described [here](https://stackoverflow.com/questions/892811/drawing-isometric-game-worlds/893063#893063)

## Mouse Picking

 - To select a unit, the mouse must be within its bounding box (dependent on unit size)
 - To select a building, the mouse must be within the tiles it occupies
 - This allows units *behind* buildings to be selected
 - Trees are targetable only if a gathered is selected
 - Units behind trees are still selectable
 - This rules out using the OpenGL pixel selection approach described [here](https://www.kamremake.com/devblog/unit-picking/), since buildings or trees would occlude units
