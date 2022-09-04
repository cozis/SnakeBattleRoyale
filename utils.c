#include "ch.h"
#include "utils.h"
#include "logger.h"
#include "display.h"

static int seed_ = 69420;

void setSeed(int seed)
{
  seed_ = seed;
}

int generateRandomInteger(void) {
  seed_ = generateRandomIntegerUsingSeed(seed_);
  return seed_;
}

int generateRandomIntegerUsingSeed(int seed)
{
  //if (seed == 0)
  //  seed = 1; // Se il seme è zero, genererà un altro zero.
  //int temp = seed * 15485863; // + chVTGetSystemTime();
  //seed = temp * temp * temp % 2038074743;

  seed = (seed * 1103515245 + 12345) % (1 << 31);
  return seed;
}

int generateRandomPositiveIntegerUsingSeed(int seed)
{
  int n = generateRandomIntegerUsingSeed(seed);
  if (n < 0)
    return -n;
  return n;
}

/* Symbol: newPosition
 *   Genera una nuova posizione sullo schermo. Coordinate
 *   che sono al di fuori dello schermo vengono considerate
 *   modulo la sua dimensione. Questo è necessario per far
 *   si che i serpenti che escono dallo schermo appaiano
 *   dal lato opposto.
 */
Position newPosition(int x, int y)
{
  const int display_w = Display_getWidth();
  const int display_h = Display_getHeight();
  int mod_x = x % (int) display_w;
  int mod_y = y % (int) display_h;
  if (mod_x < 0) mod_x += display_w;
  if (mod_y < 0) mod_y += display_h;
  return (Position) { .x = mod_x, .y = mod_y };
}

/* Symbol: newRandomPosition
 *   Genera una posizione randomica sullo schermo.
 * 
 * Nota: È importante che questa funzione abbia la proprietà
 *       di poter generare ogni possibile posizione sullo
 *       schermo. Chiamando questa funzione in un ciclo
 *       ogni possibile coordinata dovrebbe essere ritornata. 
 */
Position newRandomPosition(void)
{
  return newPosition(generateRandomInteger(),
                     generateRandomInteger());
}

/* Symbol: evaluateNextPosition
 *   Valuta la posizione di una casella adiacente
 *   a [pos] avente direzione [dir] rispetto a [pos].
 *
 * Nota: Le caselle oltre il bordo dello schermo sono
 *       considerate quelle iniziali del bordo opposto.
 */
Position evaluateNextPosition(Position pos, Direction dir)
{
  switch (dir) {
    case DIR_UP:    return newPosition(pos.x,   pos.y-1);
    case DIR_DOWN:  return newPosition(pos.x,   pos.y+1);
    case DIR_LEFT:  return newPosition(pos.x-1, pos.y);
    case DIR_RIGHT: return newPosition(pos.x+1, pos.y);
  }
  /* UNREACHABLE */
  return newPosition(0, 0); // For the warning.
}

/* Symbol: oppositeDirection
 *   Data una direzione, ritorna quella opposta.
 *
 * Nota: Le direzioni potrebbero essere ottimizzate
 *       in modo tale da avere che direzioni opposte
 *       hanno bit invertiti:
 *
 *         DIR_UP    = 0 = 00
 *         DIR_DOWN  = 3 = 11
 *         DIR_LEFT  = 2 = 10
 *         DIR_RIGHT = 1 = 01
 *
 *       In modo tale che:
 *
 *         oppositeDirection(dir) = ~dir
 */
Direction oppositeDirection(Direction dir)
{
  switch (dir) {
  case DIR_UP: return DIR_DOWN;
  case DIR_DOWN: return DIR_UP;
  case DIR_LEFT: return DIR_RIGHT;
  case DIR_RIGHT: return DIR_LEFT;
  }
  /* UNREACHABLE */
  Logger_printf("UNREACHABLE :: Invalid direction");
  return DIR_LEFT; // For the warning.
}

void delay(unsigned int ms)
{
  chThdSleepMilliseconds(ms);
}
