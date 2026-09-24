#ifndef LOCALSEARCHS_IG_HOST_H
#define LOCALSEARCHS_IG_HOST_H

#include <optional>
#include <string>
#include <vector>

#include "Instance.h"
#include "Parameters.h"
#include "Solution.h"

// Hosts the IG_IJ metaheuristic (IG_IJ/src/IG_IJ.cpp:85-161: destroy -> NEH
// reinsertion -> ONE local-search call -> SA-style accept/reject, looped
// until a wall-clock budget) with a SWAPPABLE local-search step, so all 23
// entries from local-searchs-fair's catalog can be compared not as
// standalone local searches, but as the local-search component of an actual
// metaheuristic - answering "which one makes IG_IJ converge fastest/best",
// not just "which one is fastest/best in isolation". See README.md.
namespace ig {

// One row per IG call (destroy+reconstruct+local-search+accept/reject),
// only ever collected for repetition 1 of a given (entry, instance) - see
// Parameters::trajectory_max_rows/no_trajectory.
struct CallRecord {
    size_t call_index = 0;
    size_t cost_before_ls = 0; // incumbent cost right after destroy+reconstruct, before this call's local search
    size_t cost_after_ls = 0;  // incumbent cost right after this call's local search
    bool accepted = false;     // did `current` become this call's incumbent (improve OR SA-accept)?
    size_t current_cost = 0;   // `current`'s cost AFTER this call's accept/reject decision
    size_t best_cost = 0;      // `best`'s cost AFTER this call's accept/reject decision
    double elapsed_ms = 0.0;   // wall time since the run started, right after this call's local search
};

// One row per repetition - the summary local-searchs-fair's Result played,
// but now describing a WHOLE IG run instead of a single local-search call.
struct RunResult {
    std::string name;
    size_t n = 0;
    size_t m = 0;
    size_t iteration = 0; // 1-based repetition index, out of `iters`
    size_t iters = 0;
    size_t seed = 0;
    size_t initial_cost = 0;         // cost of the PF_NEH construction, before any IG iteration
    size_t final_best_cost = 0;      // `best.cost` when the run's time budget ran out
    size_t n_calls = 0;              // how many times the local-search slot was invoked
    size_t call_of_best = 0;         // call_index at which `best` was last improved (0 = never improved past initial)
    double avg_improvement_per_call = 0.0; // mean(cost_before_ls - cost_after_ls) over ALL calls, not just logged ones
    double time_ms = 0.0;            // wall time of the whole run
    double time_limit_ms = 0.0;      // shared budget every entry ran under
    std::string note;
};

// Runs one local search (by catalog name) or all of them when `which` is
// empty, each hosted inside the IG_IJ loop above, for `params.iters()`
// repetitions. `trajectory` collects CallRecord rows for repetition 1 of
// EVERY entry run (tagged by RunResult::name via the parallel `trajectory_of`
// vector), respecting `params.trajectory_max_rows()`/`no_trajectory()`.
struct TrajectoryBlock {
    std::string name;
    std::vector<CallRecord> calls;
};

std::vector<RunResult> run(const std::optional<std::string> &which, Instance &instance, const Parameters &params,
                           size_t seed, std::vector<TrajectoryBlock> &trajectories);

// CSV: localsearch,n,m,iteration,iters,seed,initial_cost,final_best_cost,n_calls,call_of_best,avg_improvement_per_call,time_ms,time_limit_ms,note
std::string header();
std::string format(const RunResult &r);

// CSV: localsearch,call_index,cost_before_ls,cost_after_ls,accepted,current_cost,best_cost,elapsed_ms
std::string trajectory_header();
std::string format(const std::string &name, const CallRecord &c);

} // namespace ig

#endif
