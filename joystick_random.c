#include "utils.h"
#include "logger.h"
#include "joystick.h"

static Button getButton(Joystick *joystick, int player);

static JoystickMethodTable table = {
  .getButton = getButton,
  .free = 0,
};

void RandomJoystick_init2(RandomJoystick *joystick, int seed)
{
  joystick->base.table = &table;
  joystick->seed = seed;
}

void RandomJoystick_init(RandomJoystick *joystick)
{
  return RandomJoystick_init2(joystick, generateRandomInteger());
}

static Button getButton(Joystick *joystick, int player)
{
  (void) player;

  RandomJoystick *joystick2 = (RandomJoystick*) joystick;

  int random_integer = generateRandomPositiveIntegerUsingSeed(joystick2->seed);
  joystick2->seed = random_integer;

  Button generated_button = (random_integer % BUTTON_COUNT);

  Logger_printf("Random Joystick generated %s", buttonName(generated_button));

  return generated_button;
}
