A simple pong game that I will compile to emscripten
Game Data:

struct position{x, y}
struct speed {x, y}

AI for left paddle

Game engine:

Main Loop:
  
  Input Loop:
    mouse y position for player movement
    moust Click to reset game

  Logic:
    Ball movement: position + speed

    bouncing:
      top:     ball Y is lower the Ball radius, make y speed positive;
      bottom:  ball Y is higher than  screen resultion - Ball radius, make y speed negative.
      paddle:  Ball Y inside paddle, hit center -> gain speed. Hit off center -> slow X and increase Y;

    A.i.:
      paddle y = ball y. Max speed to let player actually win.

    Player:
      collide with screen

    Ball goes outside:
      left: Pause everything until player clicks

  Drawing:
    Paddle

    Ball

    Score


note:
DELETE LATER https://web.dev/drawing-to-canvas-in-emscripten/ 
