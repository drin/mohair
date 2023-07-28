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
    print(ViewQueryPlan(mohair_plan))

    # view the breakers in the mohair plan
    print(ViewQueryBreakers(mohair_plan))

    # View a naive subplan
    decomposed_plan = SplitQueryPlan(mohair_plan)

    logger.info('SubPlans:')
    for subplan_root in decomposed_plan.anchor_subplans:
        logger.info(ViewPlanOp(subplan_root))

    subplan_tree_ndx  = 0
    substrait_subplan = substrait_plan.ToSubPlanMessage(decomposed_plan, subplan_tree_ndx)
    with open(example_output, 'wb') as subplan_handle:
        subplan_handle.write(substrait_subplan.msg)
