#include "optimisations/intelligent/stalemate/goalreachable_guard.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "pieces/pieces.h"
#include "stipulation/has_solution_type.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

static boolean stalemate_are_there_sufficient_moves_left_for_required_captures(void)
{
  boolean result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CapturesLeft[nbply] = CapturesLeft[parent_ply[nbply]];
  if (TSTFLAG(move_effect_journal[capture].u.piece_removal.flags,Black))
    --CapturesLeft[nbply];

  TraceValue("%u\n",CapturesLeft[nbply]);
  result = MovesLeft[White]>=CapturesLeft[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean stalemate_isGoalReachable(void)
{
  boolean result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && target_position[GetPieceId(move_effect_journal[capture].u.piece_removal.flags)].diagram_square!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u\n",MovesLeft[Black]);

    if (nbply==2
        || move_effect_journal[top-1].type==move_effect_disable_castling_right)
    {
      square const *bnp;
      MovesRequired[White][nbply] = 0;
      MovesRequired[Black][nbply] = 0;
      for (bnp = boardnum; *bnp!=initsquare; bnp++)
      {
        square const from_square = *bnp;
        if (!is_square_empty(from_square) && !is_square_blocked(from_square))
        {
          PieceIdType const id = GetPieceId(spec[from_square]);
          if (target_position[id].diagram_square!=initsquare)
          {
            Side const from_side = TSTFLAG(spec[from_square],White) ? White : Black;
            PieNam const from_piece = get_walk_of_piece_on_square(from_square);
            MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_no_check(from_side,
                                                                                              from_piece,
                                                                                              from_square,
                                                                                              target_position[id].type,
                                                                                              target_position[id].diagram_square);
          }
        }
      }
    }
    else
    {
      move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
      PieceIdType const id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
      MovesRequired[White][nbply] = MovesRequired[White][parent_ply[nbply]];
      MovesRequired[Black][nbply] = MovesRequired[Black][parent_ply[nbply]];

      if (target_position[id].diagram_square!=initsquare)
      {
        square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
        PieNam const pi_departing = move_effect_journal[movement].u.piece_movement.moving;
        square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
        PieNam const pi_arrived = get_walk_of_piece_on_square(sq_arrival);
        Side const side_arrived = TSTFLAG(spec[sq_arrival],White) ? White : Black;
        unsigned int const time_before = intelligent_count_nr_of_moves_from_to_no_check(side_arrived,
                                                                                        pi_departing,
                                                                                        sq_departure,
                                                                                        target_position[id].type,
                                                                                        target_position[id].diagram_square);

        unsigned int const time_now = intelligent_count_nr_of_moves_from_to_no_check(side_arrived,
                                                                                     pi_arrived,
                                                                                     sq_arrival,
                                                                                     target_position[id].type,
                                                                                     target_position[id].diagram_square);

        TracePiece(pi_departing);
        TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
        TracePiece(e[move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival]);
        TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival);
        TracePiece(target_position[id].type);
        TraceSquare(target_position[id].diagram_square);
        TraceValue("%u",time_before);
        TraceValue("%u\n",time_now);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    TraceValue("%u",MovesRequired[White][nbply]);
    TraceValue("%u\n",MovesRequired[Black][nbply]);

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]
              && stalemate_are_there_sufficient_moves_left_for_required_captures());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_stalemate_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (stalemate_isGoalReachable())
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
