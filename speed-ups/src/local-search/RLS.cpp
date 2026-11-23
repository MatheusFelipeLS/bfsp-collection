#include "local-search/RLS.h"

#include "Core.h"
#include "Instance.h"
#include "Solution.h"
#include "constructions/NEH.h"
#include "local-search/Grabowski.h"
#include "Stats.h"

#include <algorithm>
#include <vector>

// Referenced Local Search (RIS).
//
// Copied from MA/src/local-search/RLS.cpp - the "full" RLS flavour shared, byte
// for byte, by MA, IG_IJ, IG_VND1, IG_VND2, DIWO, SaDIWO, MFFO, HDDE and DE_ABC.
// The only change here is that the Grabowski critical-block machinery was moved
// to local-search/Grabowski.{h,cpp} so the benchmark can reuse it.
//
// Speed ups exercised:
//   - rls           : #1 (Taillard insertion, NEH::taillard_best_insertion)
//   - rls_grabowski : #1 + #2 (block restriction) + #3 (partial recompute of the
//                     accepted move via core::partial_recalculate_solution)

namespace {

// Body shared by the ablation variants: identical to `rls` below, except that
// the best-insertion evaluator is a parameter.
template <typename BestInsertion>
bool rls_with(Solution &s, const std::vector<size_t> &ref, Instance &instance, BestInsertion best_insertion) {
    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    while (cnt < instance.num_jobs()) {
        j = (j + 1) % instance.num_jobs();

        const size_t job = ref[j];
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
            }
        }

        const auto [best_index, makespan] = best_insertion(s.sequence, job);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);

        if (makespan < s.cost) {
            cnt = 0;
            s.cost = makespan;
            improved = true;
            continue;
        }

        cnt++;
    }

    return improved;
}

} // namespace

bool rls_taillard_no_cutoff(Solution &s, const std::vector<size_t> &ref, Instance &instance) {
    NEH helper(instance);
    return rls_with(s, ref, instance, [&helper](const std::vector<size_t> &seq, size_t job) {
        return helper.taillard_best_insertion_no_cutoff(seq, job);
    });
}

bool rls_taillard(Solution &s, const std::vector<size_t> &ref, Instance &instance) {
    NEH helper(instance);
    return rls_with(s, ref, instance, [&helper](const std::vector<size_t> &seq, size_t job) {
        return helper.taillard_best_insertion(seq, job);
    });
}

bool rls_grabowski(Solution &s, const std::vector<size_t> &ref, Instance &instance) {
    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    NEH helper(instance);
    core::recalculate_solution(instance, s); // Set departure times matrix
    while (cnt < instance.num_jobs()) {
        j = (j + 1) % instance.num_jobs();

        const size_t job = ref[j];

        const auto ranges = grabowski_reinsertion(instance, s, job);

        size_t og_index = 0;
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
                og_index = i;
                break;
            }
        }

        const auto [best_index, makespan] = helper.taillard_grabowski_best_ins(s, job, ranges);

        if (makespan < s.cost) {
            s.sequence.insert(s.sequence.begin() + (long)best_index, job);
            cnt = 0;
            s.cost = makespan;
            improved = true;
            core::partial_recalculate_solution(instance, s, std::min(og_index, best_index));
            continue;
        }

        s.sequence.insert(s.sequence.begin() + (long)og_index, job);

        cnt++;
    }

    return improved;
}

// Block restriction ONLY: no Taillard head/tail matrices.
//
// Same search as rls_grabowski (critical-block ranges, strict-improvement
// acceptance, partial recompute after an accepted move), but every candidate
// position is scored by a forward recompute of the departure times from the
// insertion point to the end - O((n-i)*m) per position instead of O(m) - with
// an early cut-off: departure times on the last machine grow monotonically along
// the sequence, so a row whose last-machine departure already reaches the best
// makespan found proves the candidate cannot improve.
bool rls_grabowski_blocks(Solution &s, const std::vector<size_t> &ref, Instance &instance) {
    const size_t n = instance.num_jobs();
    const size_t m = instance.num_machines();
    auto p = [&instance](size_t i, size_t j) { return instance.p(i, j); };

    // Fills `out` with the departure times of `node` scheduled right after the row `prev`
    // (nullptr when `node` is first).
    auto fill_row = [&](const std::vector<size_t> *prev, size_t node, std::vector<size_t> &out) {
        if (prev == nullptr) {
            out[0] = p(node, 0);
            for (size_t j = 1; j < m; j++) {
                out[j] = out[j - 1] + p(node, j);
            }
            return;
        }
        const std::vector<size_t> &pr = *prev;
        out[0] = std::max(pr[0] + p(node, 0), pr[1]);
        for (size_t j = 1; j + 1 < m; j++) {
            out[j] = std::max(out[j - 1] + p(node, j), pr[j + 1]);
        }
        out[m - 1] = out[m - 2] + p(node, m - 1);
    };

    // base: departure times of the sequence without the job being moved.
    // scratch: rows of the candidate being scored.
    std::vector<std::vector<size_t>> base(n, std::vector<size_t>(m));
    std::vector<std::vector<size_t>> scratch(n, std::vector<size_t>(m));
    std::vector<size_t> reduced;
    reduced.reserve(n);

    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    core::recalculate_solution(instance, s); // departure times of the current sequence

    while (cnt < n) {
        j = (j + 1) % n;
        const size_t job = ref[j];

        const auto ranges = grabowski_reinsertion(instance, s, job);

        size_t og_index = 0;
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                og_index = i;
                break;
            }
        }

        reduced = s.sequence;
        reduced.erase(reduced.begin() + (long)og_index);

        // Rows before og_index are unchanged by the removal; recompute the rest.
        for (size_t r = 0; r < og_index; r++) {
            base[r] = s.departure_times[r];
        }
        for (size_t r = og_index; r + 1 < n; r++) {
            fill_row(r == 0 ? nullptr : &base[r - 1], reduced[r], base[r]);
        }

        stats::get().positions_total += n; // n insertion positions (reduced size + 1)
        size_t best_index = og_index;
        size_t best_value = s.cost;

        for (const auto &range : ranges) {
            stats::get().positions_evaluated += range.second - range.first + 1;
            for (size_t i = range.first; i <= range.second; i++) {
                // Candidate: reduced[0..i) + job + reduced[i..); rows i..n-1 are recomputed.
                stats::get().machines_total += (n - i) * m;
                bool pruned = false;
                const std::vector<size_t> *prev = (i == 0) ? nullptr : &base[i - 1];
                for (size_t r = i; r < n; r++) {
                    const size_t node = (r == i) ? job : reduced[r - 1];
                    fill_row(prev, node, scratch[r]);
                    prev = &scratch[r];
                    stats::get().machines_scanned += m;
                    if (scratch[r][m - 1] >= best_value) {
                        pruned = true;
                        break;
                    }
                }
                if (!pruned) { // reached the last row strictly below best_value
                    best_value = scratch[n - 1][m - 1];
                    best_index = i;
                }
            }
        }

        if (best_value < s.cost) {
            s.sequence = reduced;
            s.sequence.insert(s.sequence.begin() + (long)best_index, job);
            cnt = 0;
            s.cost = best_value;
            improved = true;
            core::partial_recalculate_solution(instance, s, std::min(og_index, best_index));
            continue;
        }

        cnt++;
    }

    return improved;
}

bool rls(Solution &s, const std::vector<size_t> &ref, Instance &instance) {

    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    NEH helper(instance);
    while (cnt < instance.num_jobs()) {
        j = (j + 1) % instance.num_jobs();

        const size_t job = ref[j];
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
            }
        }

        auto [best_index, makespan] = helper.taillard_best_insertion(s.sequence, job);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);

        if (makespan < s.cost) {
            cnt = 0;
            s.cost = makespan;
            improved = true;
            continue;
        }

        cnt++;
    }

    return improved;
}
