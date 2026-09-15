#ifndef VND_H
#define VND_H

#include <vector>

#include "Instance.h"
#include "Solution.h"

// The two-neighbourhood VND drivers (rls x best_swap_ig) and the probabilistic
// dispatcher, ported from the k-loop / branch inside solve() of IG_VND1, IG_VND2
// and IG_IJ (destroy/construct perturbation is NOT reproduced - only the local
// search step). `reference` is used as-is (never reshuffled here), matching
// how `reference = current.sequence` is passed unshuffled to `rls` in all three
// origin files.
//
// Both VNDs call best_swap_ig internally (the buggy, no-op comparison, see
// BestSwapIg.h) because that is the actual behaviour of IG_VND1/IG_VND2 today:
// vnd1's k=2 step never contributes (degrades to a single `rls` call), and
// vnd2's k=1 step is dead (degrades to a no-op followed by `rls`).

// IG_VND1/src/IG_VND1.cpp:129-143 - k=1 rls, k=2 best_swap_ig, restart k=1 on any gain.
void vnd1(Solution &incumbent, std::vector<size_t> &reference, Instance &instance);

// IG_VND2/src/IG_VND2.cpp:129-143 - k=1 best_swap_ig, k=2 rls, same restart rule.
void vnd2(Solution &incumbent, std::vector<size_t> &reference, Instance &instance);

// Not present in the repository: vnd1/vnd2 with the BestSwap comparison bug
// fixed (best_swap_ig_fixed instead of best_swap_ig), everything else
// identical (same k order, same restart rule) - added for side-by-side
// comparison against vnd1/vnd2, to measure the quality gain the VND would
// have if the swap neighbourhood actually worked.
void vnd1_fixed(Solution &incumbent, std::vector<size_t> &reference, Instance &instance);
void vnd2_fixed(Solution &incumbent, std::vector<size_t> &reference, Instance &instance);

// IG_IJ/src/IG_IJ.cpp:118-133 - NOT a VND: one probabilistic dispatch per call,
// `RNG(0,1) < jp` -> best_swap_ig, else -> rls. Returns true when the
// best_swap_ig branch was taken (used to fill the harness's `note` field).
bool ig_ij_dispatch(Solution &incumbent, std::vector<size_t> &reference, Instance &instance, double jp);

#endif
