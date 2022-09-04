#include "game.h"
#include "logger.h"
#include "display.h"
#include "joystick.h"

/* Symbol: evaluateBestDirection
 *   Data una partita avviata [game] ed un giocatore
 *   [player], questa funzione valuta la "migliore"
 *   scelta a disposizione del giocatore [player].
 *
 *   La scelta non è ottima ma basata su un algoritmo
 *   che ha un buon compromesso tra prestazioni e
 *   utilizzo di risorse.
 * 
 */
static Button evaluateBestDirection(Game *game, int player)
{
  Position apple = Game_getApplePosition(game);
  Position snake = Game_getPlayerHeadPosition(game, player);

  int would_lose_going_up = -1;
  int would_lose_going_down = -1;
  int would_lose_going_left = -1;
  int would_lose_going_right = -1;

  if (apple.x != snake.x) {
    // Valuta la distanza dalla mela andando
    // verso sinistra e verso destra.
    int distance_going_left;
    int distance_going_right;
    if (apple.x < snake.x) {
      distance_going_left  = snake.x - apple.x;
      distance_going_right = Display_getWidth() + apple.x - snake.x;
    } else {
      distance_going_left  = Display_getWidth() + snake.x - apple.x;
      distance_going_right = apple.x - snake.x;
    }

    // Scegli la direzione con la distanza
    // minore. Se però questa porterebbe a
    // perdere, scegli quella opposta.
    // Se anche la seconda scelta porterebbe
    // a perdere, delega la decisione al 
    // resto della funzione.

    would_lose_going_left = Game_wouldLoseNextUpdateIf(game, player, DIR_LEFT);
    if (distance_going_left < distance_going_right && !would_lose_going_left)
      return BUTTON_LEFT;

    would_lose_going_right = Game_wouldLoseNextUpdateIf(game, player, DIR_RIGHT);
    if (!would_lose_going_right)
      return BUTTON_RIGHT;
  }

  if (apple.y != snake.y) {
    // Valuta la distanza dalla mela andando 
    // verso l'alto o verso il basso.
    int distance_going_up;
    int distance_going_down;
    if (apple.y < snake.y) {
      distance_going_up  = snake.y - apple.y;
      distance_going_down = Display_getHeight() + apple.y - snake.y;
    } else {
      distance_going_up = Display_getHeight() + snake.y - apple.y;
      distance_going_down = apple.y - snake.y;
    }

    // Scegli la direzione pià veloce se non
    // porta a perdere, altrimenti scegli
    // quella opposta. Se anche quella opposta
    // porterebbe a perdere, non decidere ancora.

    would_lose_going_up = Game_wouldLoseNextUpdateIf(game, player, DIR_UP);
    if (distance_going_up < distance_going_down && !would_lose_going_up)
      return BUTTON_UP;

    would_lose_going_down = Game_wouldLoseNextUpdateIf(game, player, DIR_DOWN);
    if (!would_lose_going_down)
      return BUTTON_DOWN;
  }

  // Scegli una posizione che non porta a perdere.
  // Se una posizione del genere non esiste, scegli
  // di andare verso il basso.

  if (would_lose_going_left == -1 && !Game_wouldLoseNextUpdateIf(game, player, DIR_LEFT))
      return BUTTON_LEFT;

  if (would_lose_going_right == -1 && !Game_wouldLoseNextUpdateIf(game, player, DIR_RIGHT))
      return BUTTON_RIGHT;

  if (would_lose_going_up == -1 && !Game_wouldLoseNextUpdateIf(game, player, DIR_UP))
      return BUTTON_UP;
#ifndef NOLOGGING
  if (Game_wouldLoseNextUpdateIf(game, player, DIR_DOWN)) {
    Logger_printf("AI Player %d is trapped!\n", player);
  }
#endif
  return BUTTON_DOWN;
}

static Button getButton(Joystick *joystick, int player)
{
  AIJoystick *joystick2 = (AIJoystick*) joystick;
  Game *game = (Game*) joystick2->game;

  Button button = evaluateBestDirection(game, player);
  Logger_printf("AI player %d choose %s", player, buttonName(button));
  return button;
}

static JoystickMethodTable table = {
  .getButton = getButton,
  .free = 0,
};

void AIJoystick_init(AIJoystick *ai, void *game)
{
  ai->base.table = &table;
  ai->game = game;
}
