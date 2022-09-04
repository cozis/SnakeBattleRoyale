#include "logger.h"
#include "joystick.h"

Button Joystick_getButton(Joystick *joystick, int player)
{
  if (joystick->table->getButton)
    return joystick->table->getButton(joystick, player);
  return BUTTON_NULL;
}
void Joystick_free(Joystick *joystick)
{
  if (joystick->table->free)
    joystick->table->free(joystick);
}

const char *buttonName(Button button)
{
  switch (button) {
  case BUTTON_UP:    return "BUTTON_UP";
  case BUTTON_DOWN:  return "BUTTON_DOWN";
  case BUTTON_LEFT:  return "BUTTON_LEFT";
  case BUTTON_RIGHT: return "BUTTON_RIGHT";
  case BUTTON_MIDDLE: return "BUTTON_MIDDLE";
  case BUTTON_NULL:  return "BUTTON_NULL";
  default:break;
  }
  Logger_printf("Trying to print button with value %d", (int) button);
  return "???";
}
