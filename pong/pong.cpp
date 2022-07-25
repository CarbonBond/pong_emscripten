#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <cstddef>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void ballCollision(double);
void paddleMovement(double, const Uint8 *);
void winCondition(double, const Uint8 *);
void playGame(const Uint8 *);

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 600;

SDL_Window *window;
SDL_Renderer *renderer;

struct vector2d {
  float x;
  float y;
};
struct posistion {
  float x;
  float y;
};
struct score {
  int left;
  int right;
};

struct ball {
  posistion pos;
  int radius;
  vector2d speed;
};

struct paddle {
  posistion pos;
  float width;
  float height;
  vector2d speed;
};

enum gameState { delay, play, pause };

const int BALL_RADIUS = 8;
vector2d BALL_SPEED = {400., 0.};
const int PADDLE_WIDTH = 6;
const int PADDLE_HEIGHT = 70;
vector2d PADDLE_SPEED = {0., 350.};

gameState gameState = pause;

ball ball = {{WINDOW_WIDTH / 2., WINDOW_HEIGHT / 2.}, 7, BALL_SPEED};

paddle paddle_l = {
    {0., WINDOW_HEIGHT / 2.}, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_SPEED};

paddle paddle_r = {{WINDOW_WIDTH - PADDLE_WIDTH, WINDOW_HEIGHT / 2.},
                   PADDLE_WIDTH,
                   PADDLE_HEIGHT,
                   PADDLE_SPEED};

score score = {0, 0};

void redraw() {

  SDL_SetRenderDrawColor(renderer, /* RGBA: black */ 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  TTF_Font *font = TTF_OpenFont("./RetroBlocky.ttf", 60);
  SDL_Color white = {255, 255, 255};

  {
    char text[2];
    sprintf(text, "%d", score.left);
    SDL_Surface *leftScore = TTF_RenderText_Solid(font, text, white);
    SDL_Texture *leftTexture =
        SDL_CreateTextureFromSurface(renderer, leftScore);
    SDL_FreeSurface(leftScore);
    SDL_Rect leftRect = {
        .x = WINDOW_HEIGHT / 6, .y = WINDOW_HEIGHT / 10, .w = 40, .h = 60};

    SDL_RenderCopy(renderer, leftTexture, NULL, &leftRect);
    SDL_DestroyTexture(leftTexture);
  }

  {
    char text[2];
    sprintf(text, "%d", score.right);
    SDL_Surface *leftScore = TTF_RenderText_Solid(font, text, white);
    SDL_Texture *leftTexture =
        SDL_CreateTextureFromSurface(renderer, leftScore);
    SDL_FreeSurface(leftScore);
    SDL_Rect leftRect = {
        .x = WINDOW_WIDTH - 40 - WINDOW_HEIGHT / 6, .y = WINDOW_HEIGHT / 10, .w = 40, .h = 60};

    SDL_RenderCopy(renderer, leftTexture, NULL, &leftRect);
    SDL_DestroyTexture(leftTexture);
  }

  SDL_SetRenderDrawColor(renderer, /* RGBA: white */ 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_FRect l = {.x = paddle_l.pos.x,
                 .y = paddle_l.pos.y,
                 .w = paddle_l.width,
                 .h = paddle_l.height};
  SDL_FRect r = {.x = paddle_r.pos.x,
                 .y = paddle_r.pos.y,
                 .w = paddle_r.width,
                 .h = paddle_r.height};
  SDL_RenderFillRectF(renderer, &l);
  SDL_RenderFillRectF(renderer, &r);
  SDL_SetRenderDrawColor(renderer, /* RGBA: green */ 0x00, 0x80, 0x00, 0xFF);
  filledCircleRGBA(renderer, ball.pos.x, ball.pos.y, ball.radius,
                   /* RGBA: green */ 0x00, 0x80, 0x00, 0xFF);
  if (gameState == pause) {
    SDL_Surface *pauseMsg = TTF_RenderText_Solid(font, "move to start", white);
    SDL_Texture *texturePause =
        SDL_CreateTextureFromSurface(renderer, pauseMsg);
    SDL_FreeSurface(pauseMsg);
    SDL_Rect pauseRect = {.x = WINDOW_WIDTH / 4,
                          .y = WINDOW_HEIGHT / 6,
                          .w = WINDOW_WIDTH / 2,
                          .h = 60};

    SDL_RenderCopy(renderer, texturePause, NULL, &pauseRect);
    SDL_DestroyTexture(texturePause);
  }
  SDL_RenderPresent(renderer);

  TTF_CloseFont(font);
}

Uint64 ticksNow = SDL_GetTicks64();
Uint64 ticksLast = 0;
double deltaTime = 0;

bool handle_events() {
  SDL_Event event;
  SDL_PollEvent(&event);

  const Uint8 *keystate = SDL_GetKeyboardState(NULL);

  ticksNow = SDL_GetTicks64();
  deltaTime = (double)(ticksNow - ticksLast) / 1000;
  ticksLast = ticksNow;

  if (event.type == SDL_QUIT) {
    return false;
  }

  if (keystate[SDL_SCANCODE_ESCAPE]) {
    gameState = pause;
  }

  switch (gameState) {
  case delay:
    SDL_Delay(350);
    gameState = pause;
  case pause:
    playGame(keystate);
    break;
  case play:
    paddleMovement(deltaTime, keystate);
    ballCollision(deltaTime);
    winCondition(deltaTime, keystate);
    break;
  }

  redraw();
  SDL_Delay(1);
  return true;
}

void playGame(const Uint8 *keystate) {
  if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_S]) {
    gameState = play;
  }
}

void resetPositions() {
  paddle_l.pos.y = WINDOW_HEIGHT / 2. - paddle_l.height / 2;
  paddle_r.pos.y = WINDOW_HEIGHT / 2. - paddle_r.height / 2;

  ball.pos.x = WINDOW_WIDTH / 2.;
  ball.pos.y = WINDOW_HEIGHT / 2.;
}

void winCondition(double deltaTime, const Uint8 *keystate) {
  // Left scoring.
  if (ball.pos.x > WINDOW_WIDTH) {
    score.left += 1;
    paddle_l.speed.y -= 25;
    resetPositions();
    ball.speed = {-300., 0.};
    gameState = delay;
  }

  // Right scoring.
  if (ball.pos.x < 0) {
    score.right += 1;
    paddle_l.speed.y += 25;
    resetPositions();
    ball.speed = {300., 0.};
    gameState = delay;
  }
}

void paddleMovement(double deltaTime, const Uint8 *keystate) {
  if (keystate[SDL_SCANCODE_W] &&
      paddle_r.pos.y + (paddle_r.speed.y * deltaTime) > 0) {
    paddle_r.pos.y -= paddle_r.speed.y * deltaTime;
  }

  if (keystate[SDL_SCANCODE_S] &&
      paddle_r.pos.y + paddle_r.height + (paddle_r.speed.y * deltaTime) <
          WINDOW_HEIGHT) {
    paddle_r.pos.y += paddle_r.speed.y * deltaTime;
  }

  if (paddle_l.pos.y + paddle_l.height / 2 < ball.pos.y - 10 &&
      paddle_l.pos.y < WINDOW_HEIGHT) {
    paddle_l.pos.y += paddle_l.speed.y * deltaTime;
  } else if (paddle_l.pos.y + paddle_l.height / 2 > ball.pos.y + 10 &&
             paddle_l.pos.y > 0) {
    paddle_l.pos.y -= paddle_l.speed.y * deltaTime;
  }
}

void ballHitsPaddle(double deltaTime, paddle paddle) {
  if (ball.pos.x - ball.radius + (ball.speed.x * deltaTime) <
          paddle.width + paddle.pos.x && // right paddle, left ball
      ball.pos.x + ball.radius + (ball.speed.x * deltaTime) >
          paddle.pos.x && // left paddle, right ball.
      ball.pos.y + ball.radius + (ball.speed.y * deltaTime) >
          paddle.pos.y && // top paddle, bottom ball;
      ball.pos.y - ball.radius + (ball.speed.y * deltaTime) <
          paddle.height + paddle.pos.y) { // bottom paddle, top ball.

    float hitLocation = (ball.pos.y - paddle.pos.y) / paddle.height;
    if (hitLocation < 0.001) {
      hitLocation = 0.001;
    } else if (hitLocation > 1.) {
      hitLocation = 1.;
    }

    if (hitLocation < 0.4) {
      ball.speed.x = ball.speed.x * (-0.8 - hitLocation);
      float speedMultiplier = (1. + 0.4 - hitLocation);
      ball.speed.y =
          (ball.speed.y - 10 * (speedMultiplier + 10)) * speedMultiplier;
    } else if (hitLocation > 0.66) {
      ball.speed.x = ball.speed.x * (-0.8 - hitLocation + 0.6);
      float speedMultiplier = (1. + hitLocation - 0.6);
      ball.speed.y =
          (ball.speed.y + 10 * (speedMultiplier + 10)) * speedMultiplier;
    } else {
      ball.speed.x = ball.speed.x * -1.5;
      ball.speed.y = ball.speed.y * 0.8;
    }

    if (ball.speed.x > 650) {
      ball.speed.x = 650;
    } else if (ball.speed.x < -650) {
      ball.speed.x = -650;
    }

    if (ball.speed.y > 550) {
      ball.speed.y = 550;
    } else if (ball.speed.y < -550) {
      ball.speed.y = -550;
    }
  }
}

void ballCollision(double deltaTime) {
  if (ball.pos.y + ball.radius + (ball.speed.y * deltaTime) > WINDOW_HEIGHT) {
    ball.speed.y *= -1;
  }

  if (ball.pos.y - ball.radius + (ball.speed.y * deltaTime) < 0) {
    ball.speed.y *= -1;
  }

  ballHitsPaddle(deltaTime, paddle_l);

  ballHitsPaddle(deltaTime, paddle_r);

  ball.pos.x += ball.speed.x * deltaTime;
  ball.pos.y += ball.speed.y * deltaTime;
}

void run_main_loop() {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { handle_events(); }, 0, true);
#else
  while (handle_events())
    ;
#endif
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window,
                              &renderer);

  redraw();
  run_main_loop();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}
