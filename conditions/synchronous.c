#include "conditions/synchronous.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "pieces/walks/vectors.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Determine the length of a move in Synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int synchronous_measure_length(void)
{
  int result;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent>ply_retro_move)
  {
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;

    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
    move_effect_journal_index_type const parent_top = move_effect_journal_base[parent+1];

    if (movement<parent_top)
    {
      square const sq_parent_departure = move_effect_journal_get_departure_square(parent);
      square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
      numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
      numvec const diff = sq_departure-sq_arrival;
      result = diff==parent_diff;
    }
    else
      result = true;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the length of a move in Anti-synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int antisynchronous_measure_length(void)
{
  int result;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent>ply_retro_move)
  {
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;

    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
    move_effect_journal_index_type const parent_top = move_effect_journal_base[parent+1];

    if (movement<parent_top)
    {
      square const sq_parent_departure = move_effect_journal_get_departure_square(parent);
      square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
      numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
      numvec const diff = sq_departure-sq_arrival;
      result = diff==-parent_diff;
    }
    else
      result = true;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
