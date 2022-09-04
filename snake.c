#include "display.h"
#include "logger.h"
#include "snake.h"

/* Symbol: DirectionQueue
 *   Questa classe implementa una coda circolare di
 *   elementi di tipo [Direction]. È usata per 
 *   rappresentare il corpo di un serpente.
 */
typedef struct {
  Direction data[MAX_SNAKE_LEN];
  unsigned int size, head;
} DirectionQueue;

/* Symbol: Snake
 *   Questa classe (i cui metodi sono le funzioni
 *   con nome nella forma Snake_*) rappresenta lo
 *   stato di un serpente del gioco. Il suo stato
 *   è rappresentato dalla posizione assoluta della 
 *   testa e da una lista di direzioni che descrivono
 *   la posizione di ogni componente del corpo
 *   relativamente a quella precedente.
 *
 *   Siccome quando un serpente si muove, in pratica
 *   quel che succede è che viene aggiunta una
 *   parte all'inizio del serpente (nella direzione
 *   nella quale si sta muovendo) e rimossa dalla
 *   fine, la lista risulta essere una coda
 *   circolare.
 *
 *   Il serpente può essere creato con [Snake_new]
 *   (se si vuole specificare la posizione) oppure
 *   [Snake_new2] (se la posizione deve essere
 *   generata randomicamente). Una volta creato,
 *   il serpente avrà una direzione di default
 *   [DIR_LEFT]. Per far fare un passo in avanti
 *   al serpente, basta usare [Snake_Step]. 
 *   Chiamando [Snake_grow] su un serpente, al
 *   prossimo [Snake_step], la sua dimensione sarà 
 *   aumentata di un'unità.
 *
 *   Per cambiare la direzione del serpente, si
 *   usa [Snake_changeDirection].
 */
struct Snake {
  Position head;
  Direction dir;
  DirectionQueue body;
  _Bool grow;
};

/* Symbol: SnakeSlot
 *   Questa struttura è semplicemente usata per
 *   poter costruire una lista di oggetti [Snake]
 *   senza dover aggiungere un aggiuntivo campo 
 *   [next].
 */
typedef union SnakeSlot SnakeSlot;
union SnakeSlot {
  Snake snake;
  SnakeSlot *next;
};

/* Symbol: snake_pool, free_list, snake_pool_usage
 *   Queste variabili implementano l'allocatori di oggetti
 *   di tipo [Snake]. Viene tenuta traccia dei serpenti 
 *   liberi mediante una free list, dalla quale viene
 *   effettuato un pop ogni volta che è necessario
 *   allocarne uno. Inizialmente la free list non è costruita,
 *   quindi è necessario controllare la variabile 
 *   [snake_pool_usage]. Se questa è 0 e la lista vuota,
 *   allora è necessario costruirla. 
 *
 * Nota: Non è possibile allocare più [Snake] di quanti
 *       possano entrare in questa pool, quindi il limite
 *       massimo di serpenti è MAX_SNAKES.
 */
static SnakeSlot snake_pool[MAX_SNAKES];
static SnakeSlot *free_list = 0;
static int snake_pool_usage = 0;

static void DirectionQueue_init(DirectionQueue *queue)
{
  queue->size = 0;
  queue->head = 0;
}

static void DirectionQueue_push(DirectionQueue *queue, Direction dir)
{
  queue->data[queue->head] = dir;
  queue->head = (queue->head + 1) % MAX_SNAKE_LEN;

  if (queue->size < MAX_SNAKE_LEN)
    queue->size++;
}

static void DirectionQueue_pop(DirectionQueue *queue)
{
  if (queue->size > 0)
    queue->size--;
}

static unsigned int DirectionQueue_size(DirectionQueue *queue)
{
  return queue->size;
}

static Direction DirectionQueue_top(DirectionQueue *queue, unsigned int top)
{
  int i = (queue->head-1 - top) % MAX_SNAKE_LEN;
  return queue->data[i];
}

static void Snake_init(Snake *snake, int start_x, int start_y)
{
  snake->head = newPosition(start_x, start_y);
  snake->dir = DIR_LEFT;
  snake->grow = 0;

  DirectionQueue_init(&snake->body);
}

/* Symbol: Snake_new
 *   Instanzia un serpente alla posizione avente
 *   coordinate (start_x, start_y). Se il limite
 *   di sistema dei serpenti allocati è stato
 *   raggiunto, NULL è ritornato.
 */
Snake *Snake_new(int start_x, int start_y)
{
  if (free_list == 0) {
    if (snake_pool_usage == 0) {
      // È necessario costruire la freelist.
      for (int i = 0; i < MAX_SNAKES-1; ++i)
        snake_pool[i].next = snake_pool + i + 1;
      snake_pool[MAX_SNAKES-1].next = 0;
      free_list = snake_pool;
    } else {
      // Non ci sono più serpenti disponibili.
      Logger_printf("ERROR :: Couldn't allocate snake object");
      return 0;
    }
  }

  // Estrai una struttura dalla freelist 
  // ed inizializzala.
  Snake *snake = &free_list->snake;
  free_list = free_list->next;
  snake_pool_usage++;
  Snake_init(snake, start_x, start_y);
  return snake;
}

/* Symbol: Snake_new2
 *   Instanzia un serpente e posizionalo in
 *   una posizione randomica. Se il limite
 *   di sistema dei serpenti allocati è stato
 *   raggiunto, NULL è ritornato.
 */
Snake *Snake_new2(void)
{
  return Snake_new(generateRandomInteger(),
                   generateRandomInteger());
}

void Snake_free(Snake *snake)
{
  // Rimetti il serpente nella pool freelist.
  SnakeSlot *slot = (SnakeSlot*) snake;
  slot->next = free_list;
  free_list = slot;
  snake_pool_usage--;
}

/* Symbol: Snake_changeDirection
 *   Cambia la posizione del serpente. Alla prossima
 *   chiamata di [Snake_step], il serpente si muoverà
 *   in questa direzione.
 */
void Snake_changeDirection(Snake *snake, Direction new_dir)
{
  if (new_dir == oppositeDirection(snake->dir)) {
    Logger_printf("Snakes can't go backwards");
  } else {
    snake->dir = new_dir;
  }
}

/* Symbol: Snake_Step
 *   Aggiorna la posizione del serpente ed, eventualmente
 *   aumentane la dimensione.
 */
void Snake_step(Snake *snake)
{
  DirectionQueue_push(&snake->body, oppositeDirection(snake->dir));
  snake->head = evaluateNextPosition(snake->head, snake->dir);

  if (!snake->grow) {
    DirectionQueue_pop(&snake->body);
  } else {
    snake->grow = 0;
  }
}

/* Symbol: Snake_grow
 *   Segnala al serpente che alla prossima chiamata
 *   a [Snake_step] dovrà aumentare la sua lunghezza
 *   di un'unità.
 */
void Snake_grow(Snake *snake)
{
  snake->grow = 1;
}

/* Symbol: Snake_occupiesPosition
 *   Ritorna 1 se almeno una parte del serpente [snake] 
 *   occupa la posizione data [pos], 0 altrimenti.
 */
_Bool Snake_occupiesPosition(Snake *snake, Position pos)
{
  SnakeIter iter = SnakeIter_new(snake);
  do
    if (pos.x == iter.pos.x && pos.y == iter.pos.y)
      return 1;
  while (SnakeIter_next(&iter));
  return 0;
}

/* Symbol: Snake_occupiesPosition
 *   Ritorna 1 se almeno una parte del corpo del serpente 
 *   [snake] occupa la posizione data [pos], 0 altrimenti.
 *
 * Nota: Questa funzione è come [Snake_occupiesPosition],
 *       solo che ignora la testa del serpente.
 */
_Bool Snake_bodyOccupiesPosition(Snake *snake, Position pos)
{
  SnakeIter iter = SnakeIter_new(snake);
  while (SnakeIter_next(&iter))
    if (pos.x == iter.pos.x && pos.y == iter.pos.y)
      return 1;
  return 0;
}

Position Snake_getHeadPosition(Snake *snake)
{
  return snake->head;
}

Direction Snake_getDirection(Snake *snake)
{
  return snake->dir;
}

unsigned int Snake_getSize(Snake *snake)
{
  return 1 + DirectionQueue_size(&snake->body);
}

/* Symbol: SnakeIter_new
 *   Istanzia un iteratore che permette di iterare
 *   lungo le posizioni del serpente.
 *
 * Nota: L'iterazione parte dalla testa e va verso 
 *       la coda.
 */
SnakeIter SnakeIter_new(Snake *snake)
{
  SnakeIter iter;
  iter.snake = snake;
  iter.pos = snake->head;
  iter.idx = 0;
  return iter;
}

/* Symbol: SnakeIter_next
 *   Leggi il prossimo valore dell'iteratore, ossia
 *   la posizione del prossimo corpo del serpente.
 */
_Bool SnakeIter_next(SnakeIter *iter)
{
  if (iter->idx >= DirectionQueue_size(&iter->snake->body))
    return 0;

  Direction dir = DirectionQueue_top(&iter->snake->body, iter->idx);
  iter->pos = evaluateNextPosition(iter->pos, dir);
  iter->idx++;
  return 1;
}
