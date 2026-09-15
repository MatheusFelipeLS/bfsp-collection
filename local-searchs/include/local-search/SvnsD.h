#ifndef SVNS_D_LS_H
#define SVNS_D_LS_H

#include <vector>

#include "Instance.h"
#include "Solution.h"

// SVNS_D's two local searches, ported from SVNS_D/src/SVNS_D.cpp (member
// functions turned into free functions). Each function itself is still a
// SINGLE pass over `reference`, like SvnsS.h's ls1_s_swap/ls2_s_insertion -
// the "iterate to convergence/time-limit" behaviour that makes SVNS_D
// materially different from SVNS_S lives in the *driver* wrapped around these
// calls in solve() (`while (uptime()<=time_limit) { reshuffle; if
// (!LS1_D_swap(...)) break; }`), which the harness replicates around these
// two functions using a synthetic --svns-time-limit-ms budget.

// SVNS_D/src/SVNS_D.cpp:61-94. `reference` holds positions (0..n-1). Uses
// core::partial_recalculate_solution (not full) per candidate swap, plus an
// extra resync recalculation after each outer `i` (line 90 of the origin).
bool ls1_d_swap(Solution &solution, std::vector<size_t> &reference, Instance &instance);

// SVNS_D/src/SVNS_D.cpp:96-120. `reference` holds job ids.
bool ls2_d_insertion(Solution &solution, std::vector<size_t> &reference, Instance &instance);

#endif
