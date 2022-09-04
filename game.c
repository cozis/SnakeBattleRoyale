#include "game.h"
#include "snake.h"
#include "logger.h"
#include "config.h"
#include "display.h"

//#define NOLOGGING_GAME

#ifdef NOLOGGING_GAME
#undef Logger_printf
#define Logger_printf(...) {}
#endif

struct Game {
  
  // Numero del frame corrente (è usato per 
  // fare debug)
  unsigned int ticks;
  
  // Stato della partita. è 1 quando [Game_play]
  // è stato chiamato e 0 prima.
  _Bool started;
  
  unsigned int fps;
  
  // Posizione della mela. è sempre presente
  // una ed una sola mela nel gioco, e questa
  // è la sua posizione.
  Position   apple;

  // Numero di giocatori aggiunti usando
  // [Game_plugJoystick]. Una volta che la
  // partita è cominciata usango [Game_play],
  // non sarà più possibile aggiungerne di
  // nuovi.
  int player_count;

  // Questi campo compongono l'array di giocatori 
  // in formato Struct Of Array (SOA).
  _Bool          lost[MAX_PLAYERS_PER_GAME];
  Snake       *snakes[MAX_PLAYERS_PER_GAME];
  Joystick *joysticks[MAX_PLAYERS_PER_GAME];
};

/* Symbol: Game_plugJoystick
 *   Aggiunge un giocatore alla partita. Il giocatore
 *   trasmetterà i suoi comandi al gioco mediante la
 *   classe [Joystick] (che è una classe astratta).
 */
_Bool Game_plugJoystick(Game *game, Joystick *joystick)
{
  if (game->started) {
    Logger_printf("Can't add a joystick now!");
    return 0; // Non è possibile aggiungere giocatori
              // dopo aver cominciato la partita (cioè
              // dopo [Game_play]).
  }

  if (game->player_count == MAX_PLAYERS_PER_GAME) {
    // Il limite del numero di giocatori per una
    // partita è stato raggiunto.
    Logger_printf("Joystick limit reached");
    return 0;
  }

  unsigned int i = game->player_count;
  game->lost[i] = 0;
  game->snakes[i] = 0; // Il serpente relativo a questo
                       // giocatore è creato all'inizio
                       // della partita, perchè le posizioni
                       // iniziali dei serpenti dipendono
                       // da quanti serpenti ci sono in 
                       // tutto (per metterli equidistanti).
  game->joysticks[i] = joystick;
  game->player_count++;
  return 1;
}

/* Symbol: isSnakeAt
 *   Data una posizione [pos], ritorna 1 se un serpente occupa
 *   la cella, oppure 0.
 */
static _Bool isSnakeAt(Game *game, Position pos)
{
  for (int i = 0; i < game->player_count; ++i)
    if (!game->lost[i] && Snake_occupiesPosition(game->snakes[i], pos))
      return 1;
  return 0;
}

/* Symbol: Game_spawnApple
 *   Genera una nuova posizione per la mela. è da notare
 *   che siccome c'è sempre e solo una mela, questa funzione
 *   sovrascriverà la vecchia posizione. Un altro nome
 *   per questa funzione sarebbe potuto essere [Game_respawnApple].
 *
 * Nota: La posizione della mela è generata usando il generatore
 *       di numeri pseudo-casuali di default (in utils.c). Se la 
 *       posizione generata è occupata (da un serpente), allora
 *       ne sarà generata una nuova. Se il generatore di numeri
 *       casuali genera una sequenza circolare di numeri che sono
 *       relativi a posizioni tutte occupate, questa funzione
 *       non riuscirà mai a trovare una posizione e concludersi.
 */
static void Game_spawnApple(Game *game)
{
  Position pos;
  int attempt = 0; // Teniamo traccia del numero di tentativi
                   // per debuggare eventuali cicli infiniti.
  do {
    Logger_printf("(Game tick %d) Placing apple (attempt %d)", game->ticks, attempt);
    pos = newRandomPosition();
    Logger_printf("(Game tick %d) Generated apple position (%d, %d)", game->ticks, pos.x, pos.y);
    attempt++;
  } while (isSnakeAt(game, pos));

  game->apple = pos;
}

Position Game_getApplePosition(Game *game)
{
  return game->apple;
}

Position Game_getPlayerHeadPosition(Game *game, int player)
{
  return Snake_getHeadPosition(game->snakes[player]);
}

/* Symbol: Game_wouldLoseNextUpdateIf
 *   Ritorna 1 se il giocatore [player] perderebbe cambiando
 *   la direzione del serpente a [dir] nel prossimo update
 *   del gioco, 0 altrimenti.
 */
_Bool Game_wouldLoseNextUpdateIf(Game *game, int player, Direction dir)
{
  Snake   *player_snake = game->snakes[player];
  Position player_head = Snake_getHeadPosition(player_snake);

  // La posizione della testa del giocatore se 
  // al prossimo update andasse nella direzione 
  // specificata.
  Position future_player_head = evaluateNextPosition(player_head, dir);

  // Itera lungo ciascun serpente, valuta la 
  // sua posizione futura (assumendo che non
  // cambi direzione) e controlla che la testa
  // futura del giocatore non ci sbatti contro.
  for (int i = 0; i < game->player_count; ++i) {

    Snake    *opponent_snake = game->snakes[i];
    Position  opponent_head  = Snake_getHeadPosition(opponent_snake);
    Direction opponent_dir   = Snake_getDirection(opponent_snake);
    Position future_opponent_head = evaluateNextPosition(opponent_head, opponent_dir);

    // Controlla se il giocatore si scontrerebbe contro
    // il serpente del giocatore [i].

    // Si scontrerebbero se la posizione futura testa del
    // giocatore combaciasse con la futura testa del nemico
    // oppure con una delle attuali posizioni del corpo
    // nemico meno l'ultimo blocco.
    if (player_snake != opponent_snake &&
        future_player_head.x == future_opponent_head.x &&
        future_player_head.y == future_opponent_head.y)
      return 1;

    SnakeIter iter = SnakeIter_new(opponent_snake);
    for (int j = 0; j < (int) Snake_getSize(opponent_snake)-1; ++j) {
      if (iter.pos.x == future_player_head.x &&
          iter.pos.y == future_player_head.y)
        return 1; // Si scontrerebbe col corpo!!
      SnakeIter_next(&iter);
    }
  }
  return 0;
}

/* Symbol: Game_calculateAlivePlayers
 *   Restituisce il numero di giocatori che non
 *   hanno ancora perso, ossia il numero di
 *   giocatori che hanno il flag [game->lost[i]]
 *   a 0.
 */
static int Game_calculateAlivePlayers(Game *game)
{
  int lost_count = 0;
  for (int i = 0; i < game->player_count; ++i)
    lost_count += game->lost[i];
  return game->player_count - lost_count;
}

/* Symbol: Game_getFirstSnakeAlive
 *   Ritorna il primo giocatore che non ha ancora
 *   perso. La ricerca è effettuata in ordine di
 *   [Game_plugJoystick]. Se ogni giocatore ha
 *   perso o non ci sono ancora giovatori, viene
 *   ritornato -1.
 */
static int Game_getFirstSnakeAlive(Game *game)
{
  for (int i = 0; i < game->player_count; ++i)
    if (game->lost[i] == 0)
      return i;
  return -1;
}

static GameEvent Game_update(Game *game)
{
  game->ticks++;

  const unsigned int max_snake_size = Display_getWidth()
                                    * Display_getHeight();

  for (int i = 0; i < game->player_count; ++i) {

    if (game->lost[i])
      continue; // Non aggiornare lo stato dei serpenti che hanno perso.

    Snake *snake = game->snakes[i];
    Snake_step(snake);

    Position head = Snake_getHeadPosition(snake);

    Logger_printf("(Game tick %d) player %d (%d, %d), apple (%d, %d)",
                  game->ticks, i, head.x, head.y,
                  game->apple.x, game->apple.y);

    if (head.x == game->apple.x && head.y == game->apple.y) {

      // Il serpente ha mangiato la mela!

      if (Snake_getSize(snake) == max_snake_size)
        return (GameEvent) { GameEventType_WIN, i };

      Snake_grow(snake);
      Game_spawnApple(game); // Sovrascrive la mela che c'è già.
    }

    _Bool died = 0;
    for (int j = 0; j < game->player_count && !died; ++j) {
      if (!game->lost[j]) {
        Snake *snake2 = game->snakes[j];
        died = (snake == snake2)
             ? Snake_bodyOccupiesPosition(snake2, head)
             : Snake_occupiesPosition(snake2, head);
        if (died)
          Logger_printf("(Game tick %d) Snake %d died because he ate %d", game->ticks, i, j);
      }
    }

    if (died) {
      game->lost[i] = 1;

      int alive = Game_calculateAlivePlayers(game);
      Logger_printf("(Game tick %d) A snake died, "
                    "so now there are %d alive",
                    game->ticks, alive);

      // Questo serpente è appena morto. Se è una partita
      // con un solo giocatore, allora questo equivale ad
      // aver perso. Se invece i giocatori erano più di uno,
      // allora questo serpente va semplicemente rimosso e
      // gli altri possono continuare a giocare. Se poi i
      // giocatori erano più di uno e quello a morire are il
      // penultimo, allora il serpente rimanente ha vinto.

      if (alive == 1) {
        // è rimasto un solo serpente vivo, quindi
        // è il vincitore!
        int winner = Game_getFirstSnakeAlive(game);
        return (GameEvent) { GameEventType_WIN, winner };
      }

      if (alive == 0)
        // Se non sono rimasti serpenti significa
        // che c'era un solo giocatore e si è morso
        // la coda.
        return (GameEvent) { GameEventType_LOSE, -1 };
    }
  }
  return (GameEvent) { GameEventType_NOEVENT, -1 };
}

static void Game_draw(Game *game)
{
  Display_clear(0);

  // Disegna i serpenti dei giocatori.
  for (int i = 0; i < game->player_count; ++i) {

    if (game->lost[i])
      continue; // Non disegnare i serpenti che hanno perso.

    Snake *snake = game->snakes[i];

    SnakeIter iter = SnakeIter_new(snake);
    do
      Display_drawPixel(iter.pos.x, iter.pos.y, 1);
    while (SnakeIter_next(&iter));
  }

  // Disegna la mela
  Display_drawPixel(game->apple.x, game->apple.y, 1);
  Display_update();
}

static _Bool Game_init(Game *game, unsigned int fps)
{
  game->ticks = 0;
  game->started = 0;
  game->player_count = 0;
  game->fps = fps;
  Game_spawnApple(game); // Must be called after adding the snake.
  return 1;
}

typedef union GameSlot GameSlot;

union GameSlot {
  Game game;
  GameSlot *next;
};

static GameSlot game_pool[MAX_GAMES];
static GameSlot *free_list = 0;
static unsigned int game_pool_usage = 0;

Game *Game_new(unsigned int fps)
{
  if (free_list == 0) {
    if (game_pool_usage == 0) {
      // È necessario creare la freelist
      for (int i = 0; i < MAX_GAMES-1; ++i)
        game_pool[i].next = game_pool + i + 1;
      game_pool[MAX_GAMES-1].next = 0;
      free_list = game_pool;
    } else {
      Logger_printf("ERROR :: Couldn't allocate game object");
      return 0;
    }
  }

  Game *game = &free_list->game;
  free_list = free_list->next;
  game_pool_usage++;

  if (!Game_init(game, fps)) {
    Game_free(game);
    Logger_printf("ERROR :: Couldn't initialize game object");
    return 0;
  }
  return game;
}

void Game_free(Game *game)
{
  for (int i = 0; i < game->player_count; ++i)
    Snake_free(game->snakes[i]);

  GameSlot *slot = (GameSlot*) game;
  slot->next = free_list;
  free_list = slot;
  game_pool_usage--;
}

GameEvent Game_play(Game *game)
{
  if (game->started) {
    Logger_printf("ERROR :: Method play can't be called twice");
    return (GameEvent) { GameEventType_ERROR, -1 };
  }

  Display_lockResolution();

  // Aggiungi un serpente per ciascun giocatore.
  for (int i = 0; i < game->player_count; ++i) {

    Snake *snake = Snake_new2();

    if (snake == 0) {

      // Il limite dei serpenti allocabili è stato
      // raggiunto!! Non è possibile giocare al gioco.
      Logger_printf("ERROR :: Couldn't create snake object for player index %d", i);

      // Rilascia le risorse aquisite prima di fallire
      // e ritorna l'errore al chiamante.
      for (int j = 0; j < i; ++j) {
        Snake_free(game->snakes[j]);
        game->snakes[j] = 0;
      }
      Display_unlockResolution();
      return (GameEvent) { GameEventType_ERROR, -1 };
    }

    game->snakes[i] = snake;
  }

  game->started = 1;

  while (1) {

    // Gestisci l'input di ciascun giocatore.
    for (int i = 0; i < game->player_count; ++i) {

      Button button = Joystick_getButton(game->joysticks[i], i);

      Snake *snake = game->snakes[i];
      switch (button) {
      case BUTTON_UP:    Snake_changeDirection(snake, DIR_UP);    break;
      case BUTTON_DOWN:  Snake_changeDirection(snake, DIR_DOWN);  break;
      case BUTTON_LEFT:  Snake_changeDirection(snake, DIR_LEFT);  break;
      case BUTTON_RIGHT: Snake_changeDirection(snake, DIR_RIGHT); break;
      default:break;
      }
    }

    GameEvent event = Game_update(game);
    if (event.type != GameEventType_NOEVENT) {
      Display_unlockResolution();
      return event;
    }

    Game_draw(game);
    delay(1000 / game->fps);
  }
  /* UNREACHABLE */
  Display_unlockResolution();
  return (GameEvent) { GameEventType_NOEVENT, -1 };
}
