#include "conditions/anticirce/circuit_special.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STAnticirceCircuitSpecial
 */

/* Allocate a STAnticirceCircuitSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_circuit_special_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnticirceCircuitSpecial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_circuit_special_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_rebirth!=initsquare && DiaRen(spec[sq_rebirth])==sq_rebirth)
  {
    if (echecc(nbply,advers(slices[si].starter)))
      result = opponent_self_check;
    else
      result = slice_has_solution(next);
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_circuit_special_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_rebirth!=initsquare && DiaRen(spec[sq_rebirth])==sq_rebirth)
  {
    if (echecc(nbply,advers(slices[si].starter)))
      result = opponent_self_check;
    else
      result = slice_solve(next);
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
