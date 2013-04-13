#include "stipulation/constraint.h"
#include "stipulation/fork.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>


/* Allocate a STConstraintSolver slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_solver_slice(slice_index proxy_to_constraint)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_constraint);
  TraceFunctionParamListEnd();

  result = alloc_fork_slice(STConstraintSolver,proxy_to_constraint);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STConstraintTester slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_tester_slice(slice_index proxy_to_constraint)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_constraint);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STConstraintTester,proxy_to_constraint);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalConstraintTester slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_goal_constraint_tester_slice(slice_index proxy_to_constraint)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_constraint);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STGoalConstraintTester,proxy_to_constraint);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin a STContraintSolver slice off a STContraintTester sliceto add it to the
 * root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void constraint_tester_make_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (state->spun_off[slices[si].next1]!=no_slice)
  {
    state->spun_off[si] = alloc_constraint_solver_slice(stip_deep_copy(slices[si].next2));
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].next1]);
  }

  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Copy a STGoalContraintTester slice to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void goal_constraint_tester_make_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (state->spun_off[slices[si].next1]!=no_slice)
  {
    state->spun_off[si] = alloc_goal_constraint_tester_slice(stip_deep_copy(slices[si].next2));
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].next1]);
  }

  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type constraint_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].next2;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (solve(condition,length_unspecified))
  {
    case previous_move_has_solved:
    case next_move_has_solution:
      result = solve(next,n);
      break;

    case immobility_on_next_move:
    case next_move_has_no_solution:
      result = n+2;
      break;

    case previous_move_is_illegal:
      result = previous_move_is_illegal;
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_constraint_irrelevant(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (branch_find_slice(STAttackAdapter,si,stip_traversal_context_intro)==no_slice
            && branch_find_slice(STDefenseAdapter,si,stip_traversal_context_intro)==no_slice
            && branch_find_slice(STHelpAdapter,si,stip_traversal_context_intro)==no_slice
            && branch_find_slice(STGoalReachedTester,si,stip_traversal_context_intro)!=no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_constraint_if_irrelevant(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_constraint_irrelevant(slices[si].next2))
  {
    stip_traverse_structure_children_pipe(si,st);
    dealloc_slices(slices[si].next2);
    pipe_remove(si);
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor unsatisfiable_goal_checker_removers[] =
{
  { STConstraintTester,     &remove_constraint_if_irrelevant       },
  { STGoalConstraintTester, &remove_constraint_if_irrelevant       },
  { STConstraintSolver,     &remove_constraint_if_irrelevant       },
  { STEndOfBranch,          &stip_traverse_structure_children_pipe },
  { STGeneratingMoves,      &stip_structure_visitor_noop           }
};

enum
{
  nr_unsatisfiable_goal_checker_removers =
    (sizeof unsatisfiable_goal_checker_removers
     / sizeof unsatisfiable_goal_checker_removers[0])
};

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void stip_remove_irrelevant_constraints(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    unsatisfiable_goal_checker_removers,
                                    nr_unsatisfiable_goal_checker_removers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

}
