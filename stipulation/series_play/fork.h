#if !defined(STIPULATION_SERIES_PLAY_FORK_H)
#define STIPULATION_SERIES_PLAY_FORK_H

/* This module provides functionality dealing with STSeriesFork
 * stipulation slices.
 */

#include "pyslice.h"
#include "pytable.h"

/* Allocate a STSeriesFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index proxy_to_goal);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void series_fork_make_setplay_slice(slice_index si,
                                    stip_structure_traversal *st);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_fork_insert_root(slice_index si, stip_structure_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_fork_has_solution_in_n(slice_index si,
                                               stip_length_type n);

#endif
