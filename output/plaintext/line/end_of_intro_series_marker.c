#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "pydata.h"
#include "pypipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides the STOutputPlaintextLineEndOfIntroSeriesMarker
 * slice type. Slices of this type remember the end of an intro series.
 */

/* Number of move inversions up to the current move
 */
boolean is_end_of_intro_series[maxply+1];

/* Allocate a STOutputPlaintextLineEndOfIntroSeriesMarker slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_line_end_of_intro_series_marker_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextLineEndOfIntroSeriesMarker);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_line_end_of_intro_series_marker_attack(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  is_end_of_intro_series[nbply] = true;
  result = attack(next,n);
  is_end_of_intro_series[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
