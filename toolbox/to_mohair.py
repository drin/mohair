# ------------------------------
# Dependencies

# >> Standard modules
from pathlib import Path

from mohair.query.planner import ParseSubstrait, ParseQueryPlan
from mohair.query.planner import ViewQueryPlan, ViewPlanOp, ViewQueryBreakers
from mohair.query.planner import SplitQueryPlan

from mohair import CreateMohairLogger

# ------------------------------
# Module variables

logger = CreateMohairLogger('Main')

example_input = Path('resources') / 'examples' / 'average-expression.substrait'


if __name__ == '__main__':
    # parse the protobuf into a Mohair query plan
    with open(example_input, 'rb') as file_handle:
        substrait_plan = ParseSubstrait(file_handle.read())

    # construct and view the mohair structure
    mohair_plan = ParseQueryPlan(substrait_plan)
    print(ViewQueryPlan(mohair_plan))

    # view the breakers in the mohair plan
    print(ViewQueryBreakers(mohair_plan))

    # View a naive subplan
    super_plan, sub_plans = SplitQueryPlan(mohair_plan)

    logger.info('SubPlans:')
    for sub_plan in sub_plans:
        logger.info(ViewPlanOp(sub_plan.query_subplan))
