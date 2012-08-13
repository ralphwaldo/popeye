#include "conditions/circe/king_rebirth_avoider.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterCirceRebirthHandler);

  switch (st->context)
  {
    case stip_traversal_context_attack:
      attack_branch_insert_slices(si,&prototype,1);
      break;

    case stip_traversal_context_defense:
      defense_branch_insert_slices(si,&prototype,1);
      break;

    case stip_traversal_context_help:
      help_branch_insert_slices(si,&prototype,1);
      break;

    default:
      assert(0);
      break;
  }
}

static void insert_avoider(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const landing = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_fork_slice(STCirceKingRebirthAvoider,proxy);

  assert(*landing!=no_slice);
  link_to_branch(proxy,*landing);

  switch (st->context)
  {
    case stip_traversal_context_attack:
      attack_branch_insert_slices(si,&prototype,1);
      break;

    case stip_traversal_context_defense:
      defense_branch_insert_slices(si,&prototype,1);
      break;

    case stip_traversal_context_help:
      help_branch_insert_slices(si,&prototype,1);
      break;

    default:
      assert(0);
      break;
  }
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;
  slice_index const save_landing = *landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *landing = no_slice;
  insert_landing(si,st);

  stip_traverse_structure_children_pipe(si,st);

  insert_avoider(si,st);
  *landing = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(*landing==no_slice);
  *landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_king_rebirth_avoiders(slice_index si)
{
  stip_structure_traversal st;
  slice_index landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&landing);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterCirceRebirthHandler,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_king_rebirth_avoider_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  slice_index const next = ((sq_capture==prev_king_square[White][nbply]
                             || sq_capture==prev_king_square[Black][nbply])
                            ? slices[si].next2
                            : slices[si].next1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(next,n);

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
stip_length_type circe_king_rebirth_avoider_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  slice_index const next = ((sq_capture==prev_king_square[White][nbply]
                             || sq_capture==prev_king_square[Black][nbply])
                            ? slices[si].next2
                            : slices[si].next1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
