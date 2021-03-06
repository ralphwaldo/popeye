#if !defined(CONDITIONS_PATROL_H)
#define CONDITIONS_PATROL_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the conditions Patrol Chess and Ultra-Patro Chess */

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void patrol_remove_unsupported_captures_solve(slice_index si);

/* Validate an observation according to Patrol Chess
 * @return true iff the observation is valid
 */
boolean patrol_validate_observation(slice_index si);

/* Initialise solving in Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void patrol_initialise_solving(slice_index si);

/* Validate an observation according to Ultra-Patrol Chess
 * @return true iff the observation is valid
 */
boolean ultrapatrol_validate_observation(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void ultrapatrol_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Ultra-Patrol Chess
 * @param si identifies root slice of solving machinery
 */
void ultrapatrol_initialise_solving(slice_index si);

#endif
