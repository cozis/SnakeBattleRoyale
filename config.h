
#ifndef MAX_GAMES
#define MAX_GAMES 1
#endif

#ifndef MAX_PLAYERS_PER_GAME
#define MAX_PLAYERS_PER_GAME 8
#endif

#ifndef MAX_SNAKES
#define MAX_SNAKES 8
#endif

#if MAX_SNAKES < MAX_PLAYERS_PER_GAME
#warning "Non sarà possibile allocare i serpenti necessari a creare una partita col numero massimo di giocatori"
#endif

#ifndef MAX_SNAKE_LEN
#define MAX_SNAKE_LEN 32
#endif

#define NOLOGGING
