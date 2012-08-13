#include "solving/king_square.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

static void adjust(void)
{
  if (pprise[nbply]!=vide)
  {
    square const sq_capture = move_generation_stack[current_move[nbply]].capture;
    if (sq_capture==king_square[White])
      king_square[White] = initsquare;
    if (sq_capture==king_square[Black])
      king_square[Black] = initsquare;
  }

  {
    square const sq_departure = move_generation_stack[current_move[nbply]].departure;
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
    if (sq_departure==prev_king_square[White][nbply] && king_square[White]!=initsquare)
      king_square[White] = sq_arrival;
    if (sq_departure==prev_king_square[Black][nbply] && king_square[Black]!=initsquare)
      king_square[Black] = sq_arrival;
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type king_square_adjuster_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust();
  result = attack(slices[si].next1,n);

  king_square[White] = prev_king_square[White][nbply];
  king_square[Black] = prev_king_square[Black][nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type king_square_adjuster_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust();
  result = defend(slices[si].next1,n);

  king_square[White] = prev_king_square[White][nbply];
  king_square[Black] = prev_king_square[Black][nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_king_square_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STKingSquareAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
