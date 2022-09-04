#include "joystick.h"

typedef enum {
  MenuOption_CLASSIC,
  MenuOption_VERSUS,
  MenuOption_ROYALE,
} MenuOption;

MenuOption menu(Joystick *joystick);
