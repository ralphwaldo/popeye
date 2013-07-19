#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "solving/castling.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

/* Allocate a system of slices that tests whether castling has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_castling_reached_tester_system(void)
{
  slice_index result;
  slice_index castling_tester;
  Goal const goal = { goal_castling, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  castling_tester = alloc_pipe(STGoalCastlingReachedTester);
  pipe_link(castling_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,castling_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goal_castling_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
