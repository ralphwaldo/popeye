#if !defined(CONDITIONS_PROVOCATEURS_H)
#define CONDITIONS_PROVOCATEURS_H

#include "solving/machinery/solve.h"

/* This module implements the condition Provocation Chess */

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
void provocateurs_remove_unobserved_captures_solve(slice_index si);

/* Validate an observation according to Provocation Chess
 * @return true iff the observation is valid
 */
boolean provocateurs_validate_observation(slice_index si);

/* Initialise solving in Provokation Chess
 * @param si identifies the root slice of the stipulation
 */
void provocateurs_initialise_solving(slice_index si);

#endif
