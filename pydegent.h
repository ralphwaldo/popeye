#if !defined(PYDEGENT_H)
#define PYDEGENT_H

/* Implementation of the "degenerate tree" optimisation: if due to
 * options or conditions the tree of played moves degenerates to a
 * branch with few short twigs, determining whether a certain position
 * has a solution in 1 is almost as expensive whether it has a
 * solution in N.
 * The optimisation prevents us from wasting time looking for a
 * solution in all lengths from 1 to N.
 */

#include "pydirect.h"

/* Reset the degenerate tree setting
 */
void reset_degenerate_tree(void);

/* Initialize the degenerate tree setting
 * @param max_length_short only look for a solution in
 * 1..max_length_short and full length
 */
void init_degenerate_tree(stip_length_type max_length_short);

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_degenerate_tree_guards(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type
degenerate_tree_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type
degenerate_tree_direct_solve_continuations_in_n(table continuations,
                                                slice_index si,
                                                stip_length_type n);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type degenerate_tree_direct_solve_threats_in_n(table threats,
                                                           slice_index si,
                                                           stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean degenerate_tree_are_threats_refuted_in_n(table threats,
                                                 stip_length_type len_threat,
                                                 slice_index si,
                                                 stip_length_type n);

#endif
