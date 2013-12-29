#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

boolean vaulting_kings_transmuting[nr_sides];
PieNam king_vaulters[nr_sides][PieceCount];
unsigned int nr_king_vaulters[nr_sides];

static boolean is_king_vaulting[maxply+1];

void reset_king_vaulters(void)
{
  nr_king_vaulters[White] = 0;
  nr_king_vaulters[Black] = 0;
}

void append_king_vaulter(Side side, PieNam p)
{
  king_vaulters[side][nr_king_vaulters[side]] = p;
  ++nr_king_vaulters[side];
}

static boolean is_kingsquare_observed(slice_index si)
{
  Side const side = trait[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(advers(side));
  push_observation_target(king_square[side]);
  result = is_square_observed_recursive(slices[si].next2,EVALUATE(observation));
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void vaulting_kings_generate_moves_for_piece(slice_index si, PieNam p)
{
  if (p==King)
  {
    Side const side = trait[nbply];

    if (is_kingsquare_observed(temporary_hack_is_square_observed_by_non_king[side]))
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[side]; ++i)
        generate_moves_for_piece(slices[si].next1,king_vaulters[side][i]);
    }
    else if (vaulting_kings_transmuting[side])
      return; /* don't generate non-vaulting moves */
  }

  generate_moves_for_piece(slices[si].next1,p);
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result;
  Side const side_observing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (king_square[side_observing]==initsquare)
    result = is_square_observed_recursive(slices[si].next1,evaluate);
  else
  {
    is_king_vaulting[nbply] = is_kingsquare_observed(temporary_hack_is_square_observed_by_non_king[side_observing]);
    result = is_square_observed_recursive(slices[si].next1,evaluate);
    is_king_vaulting[nbply] = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_king_vaulter(Side side, PieNam walk)
{
  unsigned int i;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(walk);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_king_vaulters[side]; ++i)
    if (walk==king_vaulters[side][i])
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean vaulting_kings_enforce_observer_walk(slice_index si)
{
  boolean result;
  Side const side_observing = trait[nbply];
  square const sq_king = king_square[side_observing];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==sq_king)
  {
    if (is_king_vaulting[nbply])
    {
      if (is_king_vaulter(side_observing,observing_walk[nbply]))
      {
        PieNam const save_walk = observing_walk[nbply];
        observing_walk[nbply] = get_walk_of_piece_on_square(sq_king);
        result = validate_observation_recursive(slices[si].next1);
        observing_walk[nbply] = save_walk;

        if (!result && !vaulting_kings_transmuting[side_observing])
          result = validate_observation_recursive(slices[si].next1);
      }
      else
        result = validate_observation_recursive(slices[si].next1);
    }
    else
      result = validate_observation_recursive(slices[si].next1);
  }
  else
    result = validate_observation_recursive(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 * @param side for whom
 */
void vaulting_kings_initalise_solving(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (nr_king_vaulters[side]==0)
  {
    king_vaulters[side][0] = EquiHopper;
    nr_king_vaulters[side] = 1;
  }

  solving_instrument_move_generation(si,side,STVaultingKingsMovesForPieceGenerator);

  stip_instrument_is_square_observed_testing(si,side,STVaultingKingIsSquareObserved);

  stip_instrument_observation_validation(si,side,STVaultingKingsEnforceObserverWalk);
  stip_instrument_check_validation(si,side,STVaultingKingsEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
