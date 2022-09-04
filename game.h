#include "utils.h"
#include "joystick.h"

typedef enum {

  // Nessun evento. Questo valore non è mai
  // ritornato da [Game_play]. è utilizzato
  // solo internamente.
  GameEventType_NOEVENT,

  // è avvenuto un errore durante la partita.
  GameEventType_ERROR,

  // Questo valore è ritornato da [Game_play]
  // per partite con un solo giocatore che ha 
  // reso il serpente il più lungo possibile, 
  // oppure per partite con più giocatori in 
  // cui ne è rimasto vivo solo uno.
  GameEventType_WIN,

  // Per partite con un solo giocatore, nel
  // caso in cui il serpente muoia, la partita
  // si conclude e [Game_play] ritorna questo
  // valore.
  GameEventType_LOSE,

} GameEventType;

/* Symbol: GameEvent
 *   Struttura dati che rappresenta il risultato
 *   di una partita. La variabile [type] descrive
 *   il tipo di evento. Se [type] è [GameEventType_WIN],
 *   allora [winner] è l'indice del giocatore vincitore 
 *   (un numero non negativo), altrimenti [winner] è
 *   [-1].
 */
typedef struct {
  GameEventType type;
  int winner;
} GameEvent;

/* Symbol: Game
 *   Questa classe rappresenta lo stato interno di una partita
 *   (quali serpenti ci sono e dove, la posizione della mela ecc)
 *   Le funzioni chiamate "Game_*" sono considerati i metodi 
 *   della classe.
 *
 *   Per grandi linee l'utilizzo della classe è il seguente:
 *     1. Creare una partita con [Game_new]
 *     2. Aggiungere giocatori con [Game_plugJoystick]
 *     3. Cominciare la partita con [Game_play]
 *     4. Gestire il valore di ritorno di [Game_play]
 *        che descrive l'esito della partite.
 *     5. Distruggere l'oggetto della partita con
 *        [Game_free].
 *
 * NOTA: Le funzioni sono documentate assieme alle
 *       loro implementazioni.
 */
typedef struct Game Game;

Game     *Game_new(unsigned int fps);
GameEvent Game_play(Game *game);
void      Game_free(Game *game);
_Bool     Game_plugJoystick(Game *game, Joystick *joystick);

// Questi sono metodi che espongono lo stato del 
// gioco necessario all'intelligenza artificiale.
Position Game_getApplePosition(Game *game);
Position Game_getPlayerHeadPosition(Game *game, int player);
_Bool    Game_wouldLoseNextUpdateIf(Game *game, int player, Direction dir);
