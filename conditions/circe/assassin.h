#if !defined(CONDITIONS_CIRCE_ASSASSIN_H)
#define CONDITIONS_CIRCE_ASSASSIN_H

/* Implementation of condition Circe Assassin
 */

#include "solving/machinery/solve.h"

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
void circe_assassin_assassinate_solve(slice_index si);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean circe_assassin_all_piece_observation_tester_is_in_check(slice_index si,
                                                                Side side_attacked);

/* Initialise the solving machinery with Assassin Circe
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval to be initialised
 */
void circe_assassin_initialise_solving(slice_index si,
                                       slice_type interval_start);

#endif
