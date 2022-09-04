#include "ch.h"
#include "hal.h"
#include "menu.h"
#include "game.h"
#include "assets.h"
#include "logger.h"
#include "display.h"
#include <string.h>

#define MAX_LEN_STR 20

typedef struct {
  char s[MAX_LEN_STR];
  int x;
  int y;
} screen_elem_t;

static mutex_t mtx;
static int cursor = 0;
static screen_elem_t screen_elem[3];
static _Bool state[3];
static _Bool choosing;
static _Bool skip;

typedef struct {
  Joystick *joystick;
  MenuOption result;
} MenuData;

static THD_WORKING_AREA(waBlinkLoop, 512);
static THD_FUNCTION(blinkLoop, arg) {
  (void) arg;

  while (choosing) {
    chMtxLock(&mtx);

    if (state[cursor] == 0) {
      Display_drawText("->", screen_elem[cursor].x - 15, screen_elem[cursor].y, 0);
      state[cursor] = 1;
    }
    else {
      Display_drawText("  ", screen_elem[cursor].x - 15, screen_elem[cursor].y, 0);
      state[cursor] = 0;
    }

    if (state[(cursor+1)%3] == 1) {
      Display_drawText("  ", screen_elem[(cursor+1)%3].x - 15, screen_elem[(cursor+1)%3].y, 0);
      state[(cursor+1)%3] = 1;
    }

    if (state[(cursor+2)%3] == 1) {
      Display_drawText("  ", screen_elem[(cursor+2)%3].x - 15, screen_elem[(cursor+2)%3].y, 0);
      state[(cursor+2)%3] = 1;
    }

    chMtxUnlock(&mtx);

    Display_update();

    int i;
    for (i = 0; i < 6; ++i) {

      if (skip) {
        skip = 0;
        break;
      }
      chThdSleepMilliseconds(50);
    }
  }

  chThdExit(0);
}


static THD_WORKING_AREA(waMenuLoop, 512);
static THD_FUNCTION(menuLoop, arg) {

  chRegSetThreadName("MenuLoop");

  MenuData *data = (MenuData*) arg;
  Joystick *joystick_player1 = data->joystick;

  chMtxObjectInit(&mtx);

  Display_clear(0);
  Display_drawImage(title_image, TITLE_RES_X, TITLE_RES_Y, 41, 5, 1);
  Display_drawImage(snake_image, SNAKE_RES_X, SNAKE_RES_Y, 28, 30, 1);
  Display_update();

  //palWaitLineTimeout(LINE_BUTTON, TIME_INFINITE);
  while (true) {
    if (Joystick_getButton(joystick_player1, 0) == BUTTON_MIDDLE) {
      break;
    }

    chThdSleepMilliseconds(20);
  }

  Display_clear(0);

  strcpy(screen_elem[0].s, "CLASSIC");
  screen_elem[0].x = 43;
  screen_elem[0].y = 18;
  strcpy(screen_elem[1].s, "VERSUS");
  screen_elem[1].x = 46;
  screen_elem[1].y = 36;
  strcpy(screen_elem[2].s, "ROYALE");
  screen_elem[2].x = 52-6;
  screen_elem[2].y = 53;

  Display_drawText("MAIN MENU", 37, 0, 0);
  Display_drawText(screen_elem[0].s, screen_elem[0].x, screen_elem[0].y, 0);
  Display_drawText(screen_elem[1].s, screen_elem[1].x, screen_elem[1].y, 0);
  Display_drawText(screen_elem[2].s, screen_elem[2].x, screen_elem[2].y, 0);

  Display_update();

  while (Joystick_getButton((Joystick*) joystick_player1, 0) == BUTTON_MIDDLE);

  state[0] = 0;
  state[1] = 0;
  state[2] = 0;
  choosing = 1;
  skip = 0;

  chThdCreateStatic(waBlinkLoop, sizeof(waBlinkLoop),
                      NORMALPRIO+1, blinkLoop, NULL);

  _Bool isChoosing;
  do {
    chMtxLock(&mtx);

    switch (Joystick_getButton((Joystick*) joystick_player1, 0)) {
    case BUTTON_DOWN:
      cursor = (cursor + 1) % 3;
      skip = 1;
      break;
    case BUTTON_UP:
      cursor = (cursor + 3 - 1) % 3;
      skip = 1;
      break;

    case BUTTON_MIDDLE:
      choosing = 0;
      skip = 1;
      break;

    default:
      break;
    }

    isChoosing = choosing;

    chMtxUnlock(&mtx);

    chThdSleepMilliseconds(100);
  } while (isChoosing);
  Logger_printf("cursor=%d\n", cursor);
  switch (cursor) {
  case 0:
    data->result = MenuOption_CLASSIC;
    break;
  case 1:
    data->result = MenuOption_VERSUS;
    break;
  case 2:
    data->result = MenuOption_ROYALE;
    break;
  default:
    Logger_printf("Unexpected cursor value");
    break;
  }
  chThdExit(0);
}

MenuOption menu(Joystick *joystick)
{
  MenuData data;
  data.joystick = joystick;
  thread_t *tip = chThdCreateStatic(waMenuLoop, sizeof(waMenuLoop),
                                    NORMALPRIO, menuLoop, (void*) &data);
  chThdWait(tip);
  return data.result;
}
