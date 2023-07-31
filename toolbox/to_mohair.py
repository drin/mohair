# ------------------------------
# Dependencies

# >> Standard modules
from pathlib import Path

# >> Mohair
from mohair import CreateMohairLogger

from mohair.query.substrait import SubstraitPlan
from mohair.query.planner import ViewQueryPlan, ViewPlanOp, ViewQueryBreakers
from mohair.query.planner import SplitQueryPlan

# ------------------------------
# Module variables

logger = CreateMohairLogger('Main')

example_input  = Path('resources') / 'examples' / 'average-expression.substrait'
example_output = Path('resources') / 'examples' / 'average-expression.subplan.substrait'


# ------------------------------
# Main Logic

if __name__ == '__main__':
    # parse the protobuf into a Mohair query plan
    with open(example_input, 'rb') as query_handle:
        substrait_plan = SubstraitPlan.FromMessageBytes(query_handle.read())

    # construct and view the mohair structure
    mohair_plan = substrait_plan.ToMohair()
    plan_str    = ViewQueryPlan(mohair_plan)
    print(f'Query Plan:\n{plan_str}\n')

    # view the breakers in the mohair plan
    # print(ViewQueryBreakers(mohair_plan))

    # View a naive subplan
    decomposed_plan = SplitQueryPlan(mohair_plan)

    subplan_anchor_str = decomposed_plan.anchor_root.plan_root.ViewStr()
    print(f'SuperPlan Anchor:\n\t{subplan_anchor_str}')

    print('SubPlans:')
    for subplan_ndx, subplan_root in enumerate(decomposed_plan.anchor_subplans):
        plan_op_str = ViewPlanOp(subplan_root, indent='\t')[1:]
        print(f'\t[{subplan_ndx}]: {plan_op_str}')

    subplan_tree_ndx  = 0
    substrait_subplan = substrait_plan.ToSubPlanMessage(decomposed_plan, subplan_tree_ndx)
    with open(example_output, 'wb') as subplan_handle:
        subplan_handle.write(substrait_subplan.msg)
