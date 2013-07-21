#if !defined(PIECES_WALKS_RIDERS_H)
#define PIECES_WALKS_RIDERS_H

/* This module implements rider pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves to the square on a line segment
 * @param sq_departure common departure square of the generated moves
 * @param sq_base first square of line segment
 * @param k vector index indicating the direction of the line segment
 */
square generate_moves_on_line_segment(square sq_departure,
                                      square sq_base,
                                      vec_index_type k);

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void rider_generate_moves(square i, vec_index_type kbeg, vec_index_type kend);

#endif