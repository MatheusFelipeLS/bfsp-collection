#ifndef BEST_SWAP_IG_H
#define BEST_SWAP_IG_H

#include "Instance.h"
#include "Solution.h"

// IG_IJ/IG_VND1/IG_VND2's `BestSwap`, byte-identical in all three files
// (e.g. IG_IJ/src/IG_IJ.cpp:40-82).
//
// KNOWN BUG, reproduced faithfully here: the best-improvement check compares
// `solution.cost` (the *original*, never-mutated cost) instead of `copy.cost`
// (the cost of the swap actually being tried). Since `best_cost` starts equal
// to `solution.cost` and `solution.cost` never changes inside the function,
// the condition `solution.cost <= best_cost` is always true, so `best_i`/
// `best_j` just end up as the *last* pair scanned, and the final guard
// `best_cost < original_cost` is therefore always false. best_swap_ig NEVER
// applies a move - see the "Bugs e peculiaridades conhecidas" section of the
// README for the full trace.
void best_swap_ig(Solution &solution, Instance &instance);

// Same neighbourhood, with the comparison bug fixed (compares copy.cost
// instead of solution.cost), for side-by-side comparison against best_swap_ig.
void best_swap_ig_fixed(Solution &solution, Instance &instance);

#endif
