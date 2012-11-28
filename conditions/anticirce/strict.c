#include "conditions/anticirce/strict.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "conditions/anticirce/anticirce.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_place_reborn_strict_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_removal(move_effect_reason_anticirce_rebirth,
                                       sq_arrival);

  if (e[current_anticirce_rebirth_square[nbply]]==vide)
  {
    move_effect_journal_do_piece_addition(move_effect_reason_anticirce_rebirth,
                                          current_anticirce_rebirth_square[nbply],
                                          anticirce_current_reborn_piece[nbply],
                                          anticirce_current_reborn_spec[nbply]);
    result = solve(slices[si].next1,n);
  }
  else
  {
    current_anticirce_rebirth_square[nbply] = initsquare;
    result = slack_length-2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_strict(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticircePlaceRebornStrict);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}