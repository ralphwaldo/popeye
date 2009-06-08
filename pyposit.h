#if !defined(PYPOSIT_H)
#define PYPOSIT_H

#include "pyboard.h"
#include "pypiece.h"

/* Declarations of types and functions related to chess positions
 */

/* Array containing an element for each square plus many slack square
 * for making move generation easier
 */
typedef piece echiquier[maxsquare+4];

/* Set of flags representing characteristics of pieces (e.g. being
 * Neutral, or being Uncapturable)
 */
typedef unsigned long Flags;

/* Some useful symbols for dealing with these flags
 */

/* Enumeration type for the two sides which move, deliver mate etc.
 */
typedef enum
{
  White,
  Black,

  nr_sides,
  no_side = nr_sides
} Side;

enum
{
  BorderSpec = 0,
  EmptySpec  = BIT(Black)+BIT(White)
};

enum
{
  maxinum    = 10    /* maximum supported number of imitators */
};

typedef square imarr[maxinum]; /* squares currently occupied by imitators */

/* Structure containing the pieces of data that together represent a
 * position.
 */
typedef struct
{
    echiquier board;          /* placement of the pieces */
    Flags spec[maxsquare+4];  /* spec[s] contains flags for piece board[i]*/
    square rb;                /* placement of the white ... */
    square rn;                /* ... and black king */
    unsigned int inum;        /* number of iterators */
    imarr isquare;            /* placement of iterators */
    unsigned int nr_piece[derbla-dernoi+1]; /* number of piece kind */
} position;

/* NOTE: the number of pieces of kind p is accessed using the
 * expression nr_piece[-dernoi+p]
 */


/* Sequence of pieces corresponding to the game array (a1..h1, a2..h2
 * ... a8..h8)
 */
extern piece const PAS[nr_squares_on_board];

/* Initial game position.
 * 
 */
extern position const game_array;

/* Initialize the game array into a position object
 * @param pos address of position object
 */
void initialise_game_array(position *pos);

#endif
