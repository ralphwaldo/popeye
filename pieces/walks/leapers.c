#include "pieces/walks/leapers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a leaper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_generate_moves(square sq_departure,
                           vec_index_type kbeg, vec_index_type kend)
{
  /* generate leaper moves from vec[kbeg] to vec[kend] */
  vec_index_type k;

  for (k= kbeg; k<= kend; ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (is_square_empty(sq_arrival)
        || piece_belongs_to_opponent(sq_arrival))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  }
}
