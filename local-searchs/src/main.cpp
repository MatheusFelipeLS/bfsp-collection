#include <algorithm>
#include <iostream>
#include <numeric>

#include "Core.h"
#include "Instance.h"
#include "Parameters.h"
#include "RNG.h"
#include "Solution.h"

#include "localsearchs/Harness.h"

// Benchmark driver for every distinct local search found in the repository.
//
//   ./local-searchs <instance>                   # benchmark every local search
//   ./local-searchs <instance> -p rls-grabowski   # just one
//   ./local-searchs <instance> -n 20 -s 1         # 20 reps, RNG seed 1
//
// Prints a CSV header on stderr and, on stdout, one CSV row PER REPETITION
// of each local search (`iters` rows per entry, not pre-averaged):
//   localsearch,n,m,iteration,iters,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,note
//
// Every entry starts from the SAME random initial solution (shuffled with the
// given seed), so improvement/time are directly comparable across entries.
// See README.md for the full catalog and deduplication rules.
int main(int argc, char *argv[]) {

    Parameters params(argc, argv);
    Instance instance(params.instance_path());

    const size_t seed = params.seed().value_or(42);
    RNG::instance().set_seed(seed);

    std::vector<size_t> seq(instance.num_jobs());
    std::iota(seq.begin(), seq.end(), 0);
    std::shuffle(seq.begin(), seq.end(), RNG::instance().gen());

    Solution initial;
    initial.sequence = seq;
    core::recalculate_solution(instance, initial);

    const auto results = ls::run(params.local_search(), instance, initial, params, seed);

    std::cerr << ls::header() << '\n';
    for (const auto &r : results) {
        std::cout << ls::format(r) << '\n';
    }

    return EXIT_SUCCESS;
}
