#ifndef SWAP_H
#define SWAP_H

#include "Instance.h"
#include "Solution.h"

// Swap / exchange neighbourhood local search.
//
// These are the components that consume speed up #3 (incremental makespan
// recomputation, core::partial_recalculate_solution) together with the
// "dirty-row restore" trick (copy.departure_times[i] = s.departure_times[i]).
//
//   - swap_local_search_first : first-improvement, extracted from
//                               IG::local_search (IG/src/IG.cpp:109-144).
//   - swap_local_search_best  : best-improvement single pass, extracted from
//                               HVNS::best_swap (HVNS/src/HVNS.cpp:343-378).
//
// Both take a solution whose departure-times matrix may be stale; they refresh it
// on entry. Return true when the sequence was improved.

bool swap_local_search_first(Solution &s, Instance &instance);
bool swap_local_search_best(Solution &s, Instance &instance);

#endif
