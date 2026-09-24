#ifndef SVNS_S_LS_H
#define SVNS_S_LS_H

#include <vector>

#include "Instance.h"
#include "Solution.h"

// SVNS_S's two local searches, ported from SVNS_S/src/SVNS_S.cpp (member
// functions turned into free functions taking Instance& explicitly). Both do a
// SINGLE pass over `reference` - in the origin algorithm's solve(), they are
// each called exactly once per outer alternation step (contrast with
// SvnsD.h's LS1_D_swap/LS2_D_insertion, whose *driver* re-calls them with a
// freshly reshuffled reference until no further improvement or a time limit).

// SVNS_S/src/SVNS_S.cpp:63-92. `reference` holds positions (0..n-1), not job
// ids. Uses a full core::recalculate_solution per candidate swap.
bool ls1_s_swap(Solution &solution, std::vector<size_t> &reference, Instance &instance);

// SVNS_S/src/SVNS_S.cpp:94-117. `reference` holds job ids.
bool ls2_s_insertion(Solution &solution, std::vector<size_t> &reference, Instance &instance);

#endif
