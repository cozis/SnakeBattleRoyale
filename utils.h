#ifndef UTILS_H
#define UTILS_H

typedef enum {
  DIR_LEFT, DIR_RIGHT,
  DIR_UP,   DIR_DOWN,
} Direction;

typedef struct {
  unsigned char x, y;
} Position;

Position  newPosition(int x, int y);
Position  newRandomPosition(void);

Direction oppositeDirection(Direction dir);
Position  evaluateNextPosition(Position pos, Direction dir);

void setSeed(int seed);
int  generateRandomInteger(void);
int  generateRandomIntegerUsingSeed(int seed);
int  generateRandomPositiveIntegerUsingSeed(int seed);

void delay(unsigned int ms);

#endif /* UTILS_H */
