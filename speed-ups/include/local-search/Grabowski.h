#ifndef GRABOWSKI_H
#define GRABOWSKI_H

#include <utility>
#include <vector>

#include "Instance.h"
#include "Solution.h"

// Speed up #2 - Grabowski-Wodecki critical-block reinsertion restriction.
//
// Walks the critical path of the blocking-flowshop graph backwards, partitions it
// into NORMAL / ANTI / VERT blocks and returns the position ranges *outside* the
// critical block that contains `job` - the only positions where reinserting `job`
// can possibly improve the makespan. Feeding these ranges to
// NEH::taillard_grabowski_best_ins turns an O(n) position scan into a scan over a
// (usually much smaller) subset of positions.
//
// Extracted verbatim from the anonymous namespace of
// MA/src/local-search/RLS.cpp so that both RLS.cpp and the benchmark harness can
// share it.
std::vector<std::pair<size_t, size_t>> grabowski_reinsertion(const Instance &instance, const Solution &s, size_t job);

#endif
