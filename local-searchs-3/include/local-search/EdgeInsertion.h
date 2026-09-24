#ifndef EDGE_INSERTION_H
#define EDGE_INSERTION_H

#include <utility>
#include <vector>

#include "Instance.h"
#include "Solution.h"

// Speed up #4 - Taillard's acceleration extended to the adjacent-pair (edge)
// insertion neighbourhood, plus its single-job cousin with the identity-move
// skip. Ported verbatim from HVNS/src/HVNS.cpp (HVNS::insert_calculation,
// HVNS::taillard_best_insertion, HVNS::taillard_best_edge_insertion,
// HVNS::best_insertion, HVNS::best_edge_insertion).
//
// The class also demonstrates speed up #6: the head/tail buffers (m_inner) and
// the forward matrix (m_f) are allocated once in the constructor and rewritten on
// every call instead of being re-allocated per move.
class EdgeInsertion {
  public:
    explicit EdgeInsertion(Instance &instance);

    // All n+1 makespans for reinserting single job `job`, in O(n*m). When
    // `original_position` is a valid index that slot is skipped (identity move).
    // Pass numeric_limits::max() to consider every position.
    std::pair<size_t, size_t> taillard_best_insertion(const std::vector<size_t> &s, size_t job,
                                                      size_t original_position);

    // All makespans for reinserting the adjacent pair `jobs` as a block, in
    // O(n*m) instead of O(n*n*m).
    std::pair<size_t, size_t> taillard_best_edge_insertion(const std::vector<size_t> &s,
                                                           std::pair<size_t, size_t> &jobs, size_t original_position);

    // best-improvement local searches built on the two routines above; return
    // true when `s` was improved. `s` is refreshed on entry.
    bool single_insertion_local_search(Solution &s);
    bool edge_insertion_local_search(Solution &s);

  private:
    size_t insert_calculation(size_t i, size_t k, size_t best_value);

    Instance &m_instance;
    Solution m_inner;                     // reused head (departure_times) + tail buffers  [#6]
    std::vector<std::vector<size_t>> m_f; // reused forward matrix                         [#6]
};

#endif
