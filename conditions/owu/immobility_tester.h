#if !defined(CONDITION_OWU_IMMOBILITY_TESTER_H)
#define CONDITION_OWU_IMMOBILITY_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type owu_immobility_tester_has_solution(slice_index si);

#endif