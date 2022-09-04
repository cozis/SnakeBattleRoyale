#include "ch.h"
#include "hal.h"
#include "menu.h"
#include "game.h"
#include "assets.h"
#include "logger.h"
#include "display.h"
#include "console.h"
#include "chprintf.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

int main(void)
{
  unsigned int fps = 10;
  unsigned int x_res = 4;
  unsigned int y_res = 4;

  halInit();
  chSysInit();

  Logger_init();
  Console_init();
  Display_init();

  PhysicalJoystick *physical_joystick_0 = Console_getPhysicalJoystick(0);
  PhysicalJoystick *physical_joystick_1 = Console_getPhysicalJoystick(1);

  Display_changeResolution(x_res, y_res);
  PhysicalJoystick_setSensitivity(physical_joystick_0, SENSITIVITY_MEDIUM);
  PhysicalJoystick_setSensitivity(physical_joystick_1, SENSITIVITY_MEDIUM);

  Game *game = Game_new(fps);
  if (game == 0) {
    Logger_printf("Couldn't initialize game");
    goto loop;
  }

  RandomJoystick random_joystick_1;
  RandomJoystick random_joystick_2;
  AIJoystick ai_joystick_1;
  AIJoystick ai_joystick_2;

  switch (menu((Joystick*) physical_joystick_0)) {
  case MenuOption_CLASSIC:
    {
      Game_plugJoystick(game, (Joystick*) physical_joystick_0);
      break;
    }

  case MenuOption_VERSUS:
    {
      Game_plugJoystick(game, (Joystick*) physical_joystick_0);
      Game_plugJoystick(game, (Joystick*) physical_joystick_1);
      break;
    }

  case MenuOption_ROYALE:
    {
      RandomJoystick_init2(&random_joystick_1, 69420);
      RandomJoystick_init2(&random_joystick_2, 10000);
      AIJoystick_init(&ai_joystick_1, game);
      AIJoystick_init(&ai_joystick_2, game);
      Game_plugJoystick(game, (Joystick*) physical_joystick_0);
      Game_plugJoystick(game, (Joystick*) &random_joystick_1);
      Game_plugJoystick(game, (Joystick*) &random_joystick_2);
      Game_plugJoystick(game, (Joystick*) &ai_joystick_1);
      Game_plugJoystick(game, (Joystick*) &ai_joystick_2);
      break;
    }
  }

  GameEvent event = Game_play(game);

  Display_clear(0);
  switch (event.type) {

  char buffer[16];
  case GameEventType_WIN:
    stbsp_snprintf(buffer, sizeof(buffer), "PLAYER %d WINS", event.winner);
    Display_drawImage(win_image, WIN_RES_X, WIN_RES_Y, 58, 17, 1);
    Display_drawText(buffer, 24, 47, 0);
    break;

  case GameEventType_LOSE:
    Display_drawImage(game_over_image, GAME_OVER_RES_X, GAME_OVER_RES_Y, 19, 20, 1);
    Display_drawText("YOU LOSE", 40, 39, 0);
    break;

  case GameEventType_NOEVENT:
  case GameEventType_ERROR:
    Logger_printf("È avvenuto un errore durante il gioco");
    break;
  }
  Display_update();
  Game_free(game);

loop:
  while (1) { chThdSleepMilliseconds(1000); }

  Logger_quit(); // Non sarà mai eseguito, in effetti.
  Console_quit();
}
