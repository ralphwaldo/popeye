#include "solving/king_capture_avoider.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

typedef struct
{
    boolean own_king_capture_possible;
    boolean opponent_king_capture_possible;
} insertion_state;

static insertion_state root_state;

/* Reset king capture avoiders
 */
void king_capture_avoiders_reset(void)
{
  root_state.own_king_capture_possible = false;
  root_state.opponent_king_capture_possible = false;
}

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side without king
 */
void king_capture_avoiders_avoid_own(void)
{
  root_state.own_king_capture_possible = true;
}

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side's opponent without king
 */
void king_capture_avoiders_avoid_opponent(void)
{
  root_state.opponent_king_capture_possible = true;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  insertion_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->own_king_capture_possible)
  {
    slice_index const prototype = alloc_pipe(STOwnKingCaptureAvoider);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  if (state->opponent_king_capture_possible)
  {
    slice_index const prototype = alloc_pipe(STOpponentKingCaptureAvoider);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal_with_potential_king_capture(slice_index si,
                                                      stip_structure_traversal *st)
{
  insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->own_king_capture_possible = true;
  stip_traverse_structure_children(si,st);
  state->own_king_capture_possible = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_king_capture_avoiders(slice_index si)
{
  insertion_state state = root_state;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STGoalCounterMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STGoalDoubleMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STBrunnerDefenderFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STKingCaptureLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type own_king_capture_avoider_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_square[starter]==initsquare
      && prev_king_square[starter][parent_ply[nbply]]!=initsquare)
    result = previous_move_is_illegal;
  else
    result = solve(slices[si].next1,n);

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
stip_length_type opponent_king_capture_avoider_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_square[advers(slices[si].starter)]==initsquare)
    result = previous_move_is_illegal;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}