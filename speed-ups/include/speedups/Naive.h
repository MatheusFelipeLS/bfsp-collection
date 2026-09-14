#ifndef NAIVE_H
#define NAIVE_H

#include <utility>
#include <vector>

#include "Instance.h"
#include "Solution.h"

// Un-accelerated baselines - the "before" side of every speed up. Each one does
// the same job as its accelerated counterpart but pays the full
// core::recalculate_solution (O(n*m)) cost on every candidate move.
namespace naive {

// Baseline for #1: all n+1 insertion makespans of `job` into `seq`, each obtained
// by a full recompute. Same tie-breaking as NEH::taillard_best_insertion (first
// argmin). Returns {best_index, best_makespan}.
std::pair<size_t, size_t> best_insertion(const std::vector<size_t> &seq, size_t job, Instance &instance);

// Baseline for #1: a whole NEH construction driven by naive::best_insertion.
Solution neh_construct(std::vector<size_t> phi, Instance &instance);

// Baseline for #3: best-improvement swap over the full pair neighbourhood, every
// candidate scored with a full recompute. Mirrors swap_local_search_best.
bool swap_neighborhood_best(Solution &s, Instance &instance);

// Baseline for #4: all makespans of reinserting the adjacent pair `jobs` into
// `seq`, each by full recompute. Returns {best_index, best_makespan}.
std::pair<size_t, size_t> best_edge_insertion(const std::vector<size_t> &seq, std::pair<size_t, size_t> jobs,
                                              Instance &instance);

// Baseline for #4: edge-insertion best-improvement local search built on the
// routine above. Mirrors EdgeInsertion::edge_insertion_local_search.
bool edge_insertion_local_search(Solution &s, Instance &instance);

// Baseline for #5: departure times of appending `node` to the partial sequence
// `partial_seq`, obtained by recomputing the entire partial schedule
// (vs core::calculate_new_departure_time, which does it in O(m)).
std::vector<size_t> new_departure_time_full(const std::vector<size_t> &partial_seq, size_t node, Instance &instance);

} // namespace naive

#endif
