#ifndef HYPRR_LS_H
#define HYPRR_LS_H

#include <cstddef>
#include <vector>

#include "Instance.h"
#include "Solution.h"

// Local search of HyPRR (Teixeira, Subramanian & Kramer - "A hybrid
// population-based ruin-and-recreate algorithm for the blocking flow shop
// scheduling problem", hyprr/articles/HyPRR.pdf, Sec. 4.2).
//
// Ported from the authors' code as first committed to hyprr/ (commit 0cec09d,
// HyPRR/LocalSearch.cpp + Individual::caminhoCritico; == hyprr/versions/HyPRR2,
// which follows the paper). NOT from hyprr/versions/HyPRR, a later variant that
// shuffles the scan order and moves swap to the 2nd neighbourhood.
//
// VND over (in this order):
//   1   insertion        - LocalSearch::relocate
//   2-4 block insertion  - LocalSearch::twoOpt(x), block of x = 2, 3, 4 jobs
//   5   swap             - LocalSearch::swap (skipped when n == 500: nViz = 4)
// Each neighbourhood is ONE first-improvement sweep: positions are scanned in
// order and every accepted move is applied on the spot, the sweep continuing
// from the new solution. The VND goes back to neighbourhood 1 whenever a sweep
// improved, and stops when all of them fail.
//
// Acceptance (tie-breaking, Sec. 4.2.3): a move is accepted when it lowers the
// makespan, or keeps it and lowers T = sum_k sum_i (D[i][k] - D[i][k-1] - p),
// the total idle + blocking time. T telescopes to sum_i D[i][n] - sum(p), so
// only the last row of departure times is needed to compare it.
//
// Speed ups (Sec. 4.2.1 / 4.2.2), each switchable through Options:
//   taillard     - Wang et al. (2010) head/tail acceleration generalised to the
//                  block-insertion and swap neighbourhoods: remove the block,
//                  keep heads/tails of the reduced sequence, and score each
//                  insertion point with x rows of departures + one max over the
//                  machines (O(n^2 m) for block insertion instead of O(n^3 m));
//                  for swap(i, j) only rows i..j are recomputed and joined to
//                  the untouched tail of j+1. Early cut-off: the max over the
//                  machines stops as soon as it exceeds the current makespan.
//                  false = every candidate rebuilt and fully recomputed (the
//                  naive baseline; same scan, same decisions).
//   block_skip   - Grabowski & Pempera (2007) block property: skip a move whose
//                  two positions lie in the same block of the critical path
//                  (insertion and swap; the original never applies it to
//                  block insertion).
//   insertion_lb - Ding et al. (2016) O(1) lower bound for insertion
//                  (LocalSearch::getInsertionLowerBound): skip when delta >= 0.
//   swap_lb      - HyPRR's O(1) lower bound for swap, eqs. (8)-(10)
//                  (LocalSearch::getSwapLowerBound): skip when delta >= 0.
//                  Present in the original source but COMMENTED OUT there, so
//                  it is off in hyprr_original().
class HyprrLS {
  public:
    struct Options {
        bool taillard = true;
        bool block_skip = true;
        bool insertion_lb = true;
        bool swap_lb = false;
    };

    // What HyPRR's own code runs.
    static Options hyprr_original() { return Options{}; }
    // Full recompute per candidate, no pruning.
    static Options naive() { return Options{false, false, false, false}; }
    // Taillard only, no pruning: same decisions as naive().
    static Options taillard_only() { return Options{true, false, false, false}; }

    // Work counters for the benchmark. Bumped once per candidate move.
    struct Work {
        size_t candidates = 0;       // moves considered by a neighbourhood that can prune
        size_t evaluated = 0;        // of those, how many were NOT pruned
        size_t pruned_improving = 0; // pruned moves that would have lowered the makespan (audit only)
        bool audit = false;          // evaluate every pruned move exactly to fill pruned_improving
    };

    HyprrLS(Instance &instance, Options options);

    // Single first-improvement sweeps. Return true when the solution improved
    // (makespan or T). `s` may have stale matrices; they are rebuilt on entry.
    bool insertion(Solution &s);
    bool block_insertion(Solution &s, size_t x);
    bool swap(Solution &s);

    // Neighbourhood i in 1..5 (see above).
    bool neighborhood(Solution &s, size_t i);
    size_t n_viz() const { return m_n == 500 ? 4 : 5; }

    // Full VND (LocalSearch::runSearchTotal).
    void vnd(Solution &s);

    Work &work() { return m_work; }

  private:
    struct Block {
        int index;
        int type; // GH (normal) = 1, ST (anti) = -1, T (T-sequence) = 0
        int sm;   // machine where the block starts (0-based)
        int em;   // machine where the block ends (0-based)
        int ji;   // first position of the block (0-based)
        int jf;   // last position of the block (0-based)
        bool joined;
    };
    struct JobInfo {
        int n_blocks; // in how many blocks the position lies (1..3, original semantics)
        int b;        // block to consider
        int i1;       // first block the position lies in
    };

    void refresh(Solution &s, size_t from);
    void critical_path(const Solution &s);
    int insertion_lb(const Solution &s, int k, int i) const;
    int swap_lb(const Solution &s, int k, int i) const;
    bool pruning() const { return m_opt.block_skip || m_opt.insertion_lb || m_opt.swap_lb; }

    bool block_sweep(Solution &s, size_t x);

    void reduced_matrices(const Solution &s, size_t k, size_t x);
    size_t eval_block(const Solution &s, size_t k, size_t x, size_t pos, size_t bound);
    size_t eval_swap(const Solution &s, size_t i, size_t j, size_t bound);
    size_t last_row_sum_block(size_t x, size_t pos);
    size_t last_row_sum_swap(const Solution &s, size_t i, size_t j);

    void build_block_move(const Solution &s, size_t k, size_t x, size_t pos);
    void build_swap_move(const Solution &s, size_t i, size_t j);
    size_t probe_cost();

    void head_row(const std::vector<size_t> &prev, size_t job, std::vector<size_t> &out) const;
    void tail_row(const std::vector<size_t> &next, size_t job, std::vector<size_t> &out) const;
    static size_t row_sum(const std::vector<size_t> &row);

    Instance &m_instance;
    Options m_opt;
    size_t m_n;
    size_t m_m;
    Work m_work;

    size_t m_t_cur = 0; // sum of the last departure row of the current solution

    std::vector<size_t> m_zero;
    std::vector<std::vector<size_t>> m_rh;  // heads of the reduced sequence
    std::vector<std::vector<size_t>> m_rt;  // tails of the reduced sequence (+1 zero row)
    std::vector<std::vector<size_t>> m_buf; // scratch rows
    std::vector<size_t> m_reduced;          // reduced sequence
    Solution m_probe;                       // candidate rebuilt for full recompute

    std::vector<Block> m_blocks;
    std::vector<JobInfo> m_jobs;
};

#endif
