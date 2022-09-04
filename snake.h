#include "utils.h"

typedef struct Snake Snake;
Snake   *Snake_new(int start_x, int start_y);
Snake   *Snake_new2(void);
void     Snake_step(Snake *snake);
void     Snake_grow(Snake *snake);
void     Snake_free(Snake *snake);
unsigned int Snake_getSize(Snake *snake);
Position Snake_getHeadPosition(Snake *snake);
void     Snake_changeDirection(Snake *snake, Direction new_dir);
_Bool    Snake_occupiesPosition(Snake *snake, Position pos);
_Bool    Snake_getBodyPosition(Snake *snake, unsigned int n, Position *pos);
_Bool    Snake_occupiesPosition(Snake *snake, Position pos);
_Bool    Snake_bodyOccupiesPosition(Snake *snake, Position pos);
Direction Snake_getDirection(Snake *snake);

typedef struct {
  Snake *snake; // const?
  Position pos;
  unsigned int idx;
} SnakeIter;

SnakeIter SnakeIter_new(Snake *snake);
_Bool     SnakeIter_next(SnakeIter *iter);
