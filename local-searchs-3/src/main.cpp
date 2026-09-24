#include <iostream>

#include "Instance.h"
#include "Parameters.h"

#include "localsearchs/IgHost.h"

// Plugs every distinct local search from the catalog (see
// local-searchs-fair/README.md for the full list) into the IG_IJ
// metaheuristic's local-search slot (IG_IJ/src/IG_IJ.cpp:126-131), one at a
// time, and measures how each choice affects IG_IJ's own convergence -
// not "how good is this local search alone", but "how good does IG_IJ get
// when built around this local search". See README.md "O que está sendo
// medido".
//
//   ./local-searchs-3 <instance>                        # every entry, plugged in one at a time
//   ./local-searchs-3 <instance> -p rls-grabowski        # just one
//   ./local-searchs-3 <instance> -n 20 -s 1              # 20 IG runs, seed 1
//   ./local-searchs-3 <instance> --time-limit-ms 5000    # bigger IG time budget
//
// Prints a CSV header on stderr and, on stdout, TWO kinds of CSV rows,
// disambiguated by the first field:
//
//   summary,<localsearch>,n,m,iteration,iters,seed,initial_cost,final_best_cost,n_calls,call_of_best,
//           avg_improvement_per_call,time_ms,time_limit_ms,note
//     - one row per (entry, repetition): how the WHOLE IG run went.
//
//   trajectory,<localsearch>,call_index,cost_before_ls,cost_after_ls,accepted,current_cost,best_cost,elapsed_ms
//     - one row per IG call (destroy+reconstruct+local-search+accept/reject), repetition 1 only,
//       capped at --trajectory-max-rows (see --no-trajectory to skip entirely).
//
// Every entry starts from the SAME PF_NEH construction and runs under the
// SAME destroy size / temperature / time budget - the only thing that varies
// is which local search occupies the slot.
int main(int argc, char *argv[]) {
    Parameters params(argc, argv);
    Instance instance(params.instance_path());

    const size_t seed = params.seed().value_or(42);

    std::vector<ig::TrajectoryBlock> trajectories;
    const auto results = ig::run(params.local_search(), instance, params, seed, trajectories);

    std::cerr << "summary_header," << ig::header() << '\n';
    std::cerr << "trajectory_header," << ig::trajectory_header() << '\n';

    for (const auto &r : results) {
        std::cout << "summary," << ig::format(r) << '\n';
    }
    for (const auto &block : trajectories) {
        for (const auto &call : block.calls) {
            std::cout << "trajectory," << ig::format(block.name, call) << '\n';
        }
    }

    return EXIT_SUCCESS;
}
