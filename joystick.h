
#ifndef JOYSTICK_H_
#define JOYSTICK_H_

typedef enum {
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_MIDDLE,
  BUTTON_NULL,
} Button;

#define BUTTON_COUNT 6

typedef struct Joystick Joystick;

typedef struct {
  Button (*getButton)(Joystick*, int);
  void   (*free)(Joystick*);
} JoystickMethodTable;

/* Symbol: Joystick
 *   Questa è la classe base di ogni oggetto che 
 *   implementa le funzionalità di un joystick.
 *
 *   Al momento, un joystick può essere un oggetto
 *   che rappresenta un joystick fisico (PhysicalJoystick
 *   implementato in "console.c") oppure un joystick
 *   simulato virtualmente (RandomJoystick e AIJoystick
 *   rispettivamente da joystick_random.c e joystick_ai.c).
 */
struct Joystick {
  JoystickMethodTable *table;
};

typedef struct {
  Joystick base;
  int seed;
} RandomJoystick;

typedef struct {
  Joystick base;
  void    *game;
} AIJoystick;

void  AIJoystick_init(AIJoystick *ai, void *game);
void  RandomJoystick_init(RandomJoystick *joystick);
void  RandomJoystick_init2(RandomJoystick *joystick, int seed);

Button Joystick_getButton(Joystick *joystick, int player);
void   Joystick_free(Joystick *joystick);

const char *buttonName(Button button);

#endif /* JOYSTICK_H_ */
