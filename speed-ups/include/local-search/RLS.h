#ifndef RLS_H
#define RLS_H

#include <vector>

#include "Instance.h"
#include "Solution.h"

bool rls(Solution &s, const std::vector<size_t> &ref, Instance &instance);
// Ablation ladder for the RLS benchmark. Same search as `rls` (same reference
// order, tie-breaking, acceptance and stop rule, hence the same trajectory); only
// the evaluation of the insertion positions changes:
//   naive::rls             - full O(n*m) recompute per position
//   rls_taillard_no_cutoff - Taillard e/q, every position scanned on all m machines
//   rls_taillard           - Taillard e/q + early cut-off (== `rls`)
bool rls_taillard_no_cutoff(Solution &s, const std::vector<size_t> &ref, Instance &instance);
bool rls_taillard(Solution &s, const std::vector<size_t> &ref, Instance &instance);

// Block restriction only (no Taillard e/q): each allowed position is scored by a
// forward recompute from the insertion point, with early cut-off and partial
// recompute after accepted moves.
bool rls_grabowski_blocks(Solution &s, const std::vector<size_t> &ref, Instance &instance);

bool rls_grabowski(Solution &s, const std::vector<size_t> &ref, Instance &instance);

#endif
